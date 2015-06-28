#include <stdlib.h>  // Is needed for exiting early if an error occurs
#include <stdio.h>   // Is needed for printing the final results
#include <math.h>	 // Is needed for fabs()/absolute value of floating point numbers
#include <string.h>  
#include <mpi.h>     // Is needed for MPI parallelization
#define SEED 35791246

int main (int argc, char **argv){
    double start_time, end_time, time_diff;
    int num_iteration = 0;
    double x, y; 			//x,y value for the random coordinate
    int i;
    int count; // Number of points in the first quadrant of unit circle
    double z;
    double pi=0.0; //holds approx value of pi
    double  pi_tot; //holds final approx value of pi

    // MPI Initialization part
    int pRank;  //every process will have a unique integer id/rank in the pool
    int pNum;   // How many processes are in the pool
    // MPI_Init(...) starts the parallel regions, here on the part of the code until
    // MPI_Finalize(...) call will be executed by all processes in the pool
    MPI_Init(&argc, &argv);
    //Next line will setup the number of processes in the communicator/pool. besed on -n ##
    MPI_Comm_size(MPI_COMM_WORLD, &pNum);
    //Next line will give a unique id to each process in the communicator/pool of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &pRank);

    if (pRank == 0) { // if I am process 0, then ask the user for number of itterations
        printf("Please enter the number of iterations used to compute pi: \n");
        scanf("%d",&num_iteration);
    }
    //Sending the number of iterations to all process in the communicator
    MPI_Bcast(&num_iteration, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // Record the start time from here on:
    start_time = MPI_Wtime();

    srandom(SEED);		//Give srandom() a seed value
    count = 0;

    for(i=pRank+1; i<=num_iteration; i+=pNum){
        x = (double)random()/RAND_MAX; //get a random x coordinate
        y = (double)random()/RAND_MAX; //get a random y coordinate
        z = (x*x) + (y*y); 			   //Check to see if number is inside unit circle
        if (z <=1) count++; 		   //if it is, consider it a valid random point
    }
    pi=((double)count/(double)num_iteration)*4.0;  //p = 4(m/n)

    //after each process is done with calculating their partial sum. the process 0 will ask all
    // other proceeses in the pool for their partial sums, and add them together to make a total
    // sum/value of pi. Reduce function is not working yet.
    //MPI_Reduce(&pi, &pi_tot, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    //End recording the time here.
    end_time = MPI_Wtime();
    time_diff = end_time - start_time;

    if (pRank == 0){ // If I am process 0, then print  out the messages

        printf("Number of iterations is = %d \n\n", num_iteration);
        printf("Estimated value of PI is %g - (%17.15f)\n\n",pi,pi);
        printf("Accurate value of PI from math.h is: %17.15f \n\n", M_PI);

        printf("Difference between computed pi and math.h M_PI = %17.15f\n\n",
               fabs(pi - M_PI));

        printf("Time to compute = %f seconds\n\n", time_diff);
    }
    //Line is for end of parallel regions of the code.
    MPI_Finalize();

    return EXIT_SUCCESS;
}
