#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define NCA 8           // Number of columns for matrix A
#define NRA 8           // Number of rows for matrix A
#define NCB 8           // Number of columns for matrix B
#define NRB 8           // Number of rows for matrix B

int main(int argc, char *argv[])
{
        char i;
        int iA, jA, iB, jB, procA, procB;
        char a[NCA * NRA];
        char b[NCB * NRB];
        const int NPRA = 2;             // Number of rows in matrix A in decompostion
        const int NPCA = 2;             // Number of columns in matrix A in decompostion
        const int NPRB = 2;             // Number of rows in matrix B in decompostion
        const int NPCB = 2;             // Number or columns in matrix B in decomposition       

        const int BLRA = NRA / NPRA;    // Number of rows in matrix A in block
        const int BLCA = NCA / NPCA;    // Number of columns in matrix A in block
        const int BLRB = NRB / NPRB;    // Number of rows in matrix B in block
        const int BLCB = NCB / NPCB;    // Number of columns in matrix A in block

        int size, rank;

        // Initialize MPI enviornment
        MPI_Init(&argc, &argv);

        // Get the number of processors
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        // Get the name of processors
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);








        // If master node
        if(rank == 0)
        {
                for(iA = 0; iA < NCA * NRA; iA++)
                {
                        a[iA] = (char)iA;
//                      printf("A: %d\n", a[iA]);
                }
                for(iB = 0; iB < NCB * NRB; iB++)
                {
                        b[iB] = (char)iB;
//                      printf("B: %d\n" , a[iB]);
                }
        }

        // Create blocks for slave nodes
        // Set each block equal to 0
        char ablock[BLRA * BLCA];
        char bblock[BLRB * BLCB];
        for(iA = 0; iA < BLRA * BLCA; iA++)
        {
                ablock[iA] = 0;
        }
        for(iB = 0; iB < BLRB * BLCB; iB++)
        {
                bblock[iB] = 0;
        }

        MPI_Datatype blocktypeA;
     MPI_Datatype blocktype2A;
        MPI_Datatype blocktypeB;
        MPI_Datatype blocktype2B;











        // Input paramaters to MPI_Type_vector
        // BLRA/B are the number of blocks (nonnegative intergers)
        // BLCA/B are the number of elements in each block (nonnegative integer)
        // NCA/B are the number of elements between start of each block(integer)
        // MPI_CHAR is teh old datatype
        // Output Parameters
        // blocktype2A/B is the new datatype
        // The purpose is to create a vector (strided) datatype
        MPI_Type_vector(BLRA, BLCA, NCA, MPI_CHAR, &blocktype2A);
//      MPI_Type_vector(BLRB, BLCB, NCB, MPI_CHAR, &blocktype2B);

        // Input Parameters to MPI_Type_create_resized
        // blocktype2A/B is teh input datatype
        // 0 is the new lower bound of datatype
        // sizeof(char) is the new extent of datatype
        // Output parameters
        // blocktypeA/B is the output datatype
        // The purpose is to create a datatype with a new lower bound and extent from an exisitng datatype
        MPI_Type_create_resized(blocktype2A, 0, sizeof(char), &blocktypeA);
//      MPI_Type_create_resized(blocktype2B, 0, sizeof(char), &blocktypeB);

        // Input parameteres to MPI_Type_commit
        // blocktypeA/B is the datatype
        // The purpose is to commit the datatype
        MPI_Type_commit(&blocktypeA);
//      MPI_Type_commit(&blocktypeB);   

        int dispsA[NPRA * NPCA];
        int dispsB[NPRB * NPCB];
        int countA[NPRA * NPCA];
        int countB[NPRB * NPCB];

        for(iA = 0; iA < NPRA; iA++)
        {
                for(jA = 0; jA < NPCA; jA++)
                {
                        dispsA[iA * NPCA + jA] = iA * NCA * BLRA + jA * BLCA;
//                      printf("A1: %d\n", dispsA[iA * NPCA + jA]);
                        countA[iA * NPCA + jA] = 1;
//                      printf("ACount: %d\n", countA[iA * NPCA + jA]);
                }
        }
        for(iB = 0; iB < NPRB; iB++)
        {
                for(jB = 0; jB < NPCB; jB++)
                {
                        dispsB[iB * NPCB + jB] = iB * NCB * BLRB + jB * BLCB;
//                      printf("B1: %d\n", dispsB[iB * NPCB + jB]);
                        countA[iB * NPCB + jB] = 1;
                }
        }

        // Input Parameters for MPI_Scarrerv
        // a is the address of send buffer
        // counts is the integer array specifying the number of elements to send to each processor
        // disps is the integer array.. Entry i specifies the displacement
        // blocktype is teh data type of send buffer elements
        // BLOCKROWS * BLOCKCOLS is the data type of receive buffer
        // MPI_CHAR is the data type of receive buffer elements
        // 0 is the rank of sending process
        // MPI_COMM_WORLD is the communicator
        // Output parameters
        // b is the address of receive buffer
        // The purpose is to scatter a buffer in parts to all processes in a communicator

        MPI_Scatterv(a, countA, dispsA, blocktypeA, ablock, BLRA * BLCA, MPI_CHAR, 0, MPI_COMM_WORLD);
        MPI_Scatterv(b, countB, dispsB, blocktypeB, bblock, BLRB * BLCB, MPI_CHAR, 0, MPI_COMM_WORLD);

        // Each procA prints it's "ablock" out in order
        for(procA = 0; procA < size / 2; procA++)
        {
                if(procA == rank)
                {
                        printf("Rank = %d\n", rank);
                        if(rank == 0)
                        {
                                printf("Global matrix: \n");
                                for(iA = 0; iA < NRA; iA++)
                                {
                                        for(jA = 0; jA < NCA; jA++)
                                        {
                                                printf("%3d ", (int)a[iA * NCA + jA]);
                                        }
                                        printf("\n");
                                }
                        }
                        printf("Local Matrix:\n");
                        for(iA = 0; iA < BLRA; iA++)
                        {
                                for(jA = 0; jA < BLCA; jA++)
                                {
                                        printf("%3d ",(int)ablock[iA * BLCA + jA]);
                                }
                                printf("\n");
                        }
                        printf("\n");
                }
                // Clocks until all processes in the communicator have reached this routine.
                MPI_Barrier(MPI_COMM_WORLD);

        }

        for(procB = size / 2; procB < size; procB++)
        {
                if(procB == rank)
                {
                        printf("Rank = %d\n", rank);
                        if(rank == size / 2)
                        {
                                printf("Global matrix: \n");
                                for(iB = 0; iB < NRB; iB++)
                                {
                                        for(jB = 0; jB < NCB; jB++)
                                        {
                                                printf("%3d ", (int)b[iB * NCB + jB]);
                                        }
                                        printf("\n");
                                }

         }
                // Clocks until all processes in the communicator have reached this routine.
                MPI_Barrier(MPI_COMM_WORLD);
        }

        // Finalize the MPI environment
        MPI_Finalize();

}



