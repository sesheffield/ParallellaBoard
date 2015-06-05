#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define COLS 24
#define ROWS 16 

void printMat(int *a,int rows, int cols)
{
  for(int i = 0; i < rows; i++)
    {
      for(int j = 0; j < cols; j++)
        {
          printf("%d ", a[i*cols+ j]);
        }
      printf("\n");
    }
  printf("\n");
}

int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);
    int p, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    //    int i;

    int a[ROWS*COLS];
    int b[ROWS*COLS];

    const int NPROWS=2;
    const int NPCOLS=4;
    const int BLOCKROWS=ROWS/NPROWS;
    const int BLOCKCOLS=COLS/NPCOLS;

    if(rank == 0)
      {
        for(int i = 0; i < ROWS * COLS; i++)
          {
            a[i] = i;
            b[i] = i * 2;
          }
      }

    if(p != NPROWS * NPCOLS)
      {
        fprintf(stderr,"Err: number of PEs: %d is not %d x %d\n", p, NPROWS, NPCOLS);
        MPI_Finalize();
        exit(-1);
      }
    int aOut[BLOCKROWS * BLOCKCOLS];
    int bOut[BLOCKROWS * BLOCKCOLS];

    for (int i = 0; i < BLOCKROWS * BLOCKCOLS; i++) 
      {
        aOut[i] = 0;
        bOut[i] = 0;
      }

    MPI_Datatype blocktypeA;
    MPI_Datatype blocktypeA2;
    MPI_Datatype blocktypeB;
    MPI_Datatype blocktypeB2;

    MPI_Type_vector(BLOCKROWS, BLOCKCOLS, COLS, MPI_INT, &blocktypeA2);
    MPI_Type_create_resized(blocktypeA2, 0, sizeof(int), &blocktypeA);
    MPI_Type_commit(&blocktypeA);

    MPI_Type_vector(BLOCKROWS, BLOCKCOLS, COLS, MPI_INT, &blocktypeB2);
    MPI_Type_create_resized(blocktypeB2, 0, sizeof(int), &blocktypeB);
    MPI_Type_commit(&blocktypeB);

    int dispsA[NPROWS * NPCOLS];
    int dispsB[NPROWS * NPCOLS];
    int countsA[NPROWS * NPCOLS];
    int countsB[NPROWS * NPCOLS];

    for (int i = 0; i < NPROWS; i++)
      {
        for(int j = 0; j < NPCOLS; j++)
          {
            dispsA[i * NPCOLS + j] = i * COLS * BLOCKROWS + j * BLOCKCOLS;
            dispsB[i * NPCOLS + j] = i * COLS * BLOCKROWS + j * BLOCKCOLS;
            countsA[i * NPCOLS + j] = 1;
            countsB[i * NPCOLS + j] = 1;
          }
      }

    MPI_Scatterv(a, countsA, dispsA, blocktypeA, aOut, BLOCKROWS * BLOCKCOLS, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(b, countsB, dispsB, blocktypeB, bOut, BLOCKROWS * BLOCKCOLS, MPI_INT, 0, MPI_COMM_WORLD);

    for(int proc = 0; proc < p; proc++)
      {
        if (proc == rank)
          {
            printf("Rank: %d\n", rank);
            if(rank == 0)
              {
                printf("Global matrices: \n");
                printMat(a,ROWS,COLS);
                printMat(b,ROWS,COLS);
              }
//            printf("Local matrix:\n");
            printf("Local A:\n");
            printMat(aOut,BLOCKROWS,BLOCKCOLS);
            printf("Local B:\n");
            printMat(bOut,BLOCKROWS,BLOCKCOLS);
          }
        MPI_Barrier(MPI_COMM_WORLD);
      }
      
clock_t begin, end;
double time_spent;
begin = clock();
      
      for(i = 0; i < BLOCKROWS; i++)
{
        for(j = 0; j < BLOCKCOLS; j++)
        {
                cOut[i + j * BLOCKROWS] = 0;
                for(k = 0; k < BLOCKROWS; k++)
                {
                        cOut[i + j * BLOCKROWS] += aOut[i + k * BLOCKROWS] * bOut[k + BLOCKCOLS * j];
                }
        }
}

end = clock();
time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
printf("Total time: %f\n", time_spent);


    MPI_Gatherv(cOut, BLOCKROWS * BLOCKCOLS, MPI_INT, c, countsB, dispsB, blocktypeB, 0, MPI_COMM_WORLD);


    printf("Rank: %d\n", rank);
            if(rank == 0)
              {
                printf("Global matrices: \n");
                printMat(c,ROWS,COLS);
              }


    MPI_Finalize();

    return 0;
}
