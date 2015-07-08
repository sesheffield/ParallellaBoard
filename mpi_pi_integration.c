#include <stdio.h>    // Is needed for printing the final results
#include <stdlib.h>   // Is needed for exiting early if an error occurs
#include <math.h>     // Is needed for fabs()/absolute value of floating point numbers
#include <mpi.h> 	 // Is needed for MPI parallelization

double pi_serial(int num_iter);

int main(int argc, char *argv[]) {
    int num_sub_intervals = 0;  	// number of sub intervals
    double start_time, end_time, time_diff; // for timeing most computation intese part of the program
    double x, pi, pi_tot;	   // pi=value of pi, pi_total= total value of pi after reduction
    double sum = 0.0; // for partial sum
    double step; 
    int i;

    // MPI Initialization part
    int pRank;	//every process will have a unique integer id/rank in the pool
    int pNum; 	// How many processes are in the pool
    // MPI_Init(...) starts the parallel regions, here on the part of the code until
    // MPI_Finalize(...) call will be executed by all processes in the pool
    MPI_Init(&argc, &argv);
    //Next line will setup the number of processes in the communicator/pool. besed on -n ##
    MPI_Comm_size(MPI_COMM_WORLD, &pNum);
    //Next line will give a unique id to each process in the communicator/pool of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &pRank);

    if(pRank == 0){  // If I am process 0 then ask the user for an input (number of intervals)
        printf("Please enter the number of iterations used to compute pi: \n");
        scanf("%d", &num_sub_intervals);
    }
    //Sending the number of sub intervals to all process in the communicator
    MPI_Bcast(&num_sub_intervals, 1, MPI_INT, 0, MPI_COMM_WORLD);

    step = 1.0/(double) num_sub_intervals;  

    // Record the start time from here on:
    start_time = MPI_Wtime();

    for(i=pRank+1; i<=num_sub_intervals; i+=pNum){
        x = step*((double)i-0.5);
        sum += 4.0/(1.0+x*x);
    }
    pi = step * sum;

    //after each process is done with calculating their partial sum. the process 0 will ask all
    // other proceeses in the pool for their partial sums, and add them together to make a total
    // sum/value of pi.
    MPI_Reduce(&pi, &pi_tot, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    //End recording the time here.
    end_time = MPI_Wtime();
    time_diff = end_time - start_time;

    if(pRank == 0){ //If I am process 0, prints out the results to user.
        // print the result here:
        printf("computed pi value in parallel is = %g (%17.15f)\n\n", pi_tot,pi_tot);
        printf("M_PI accurate value from math.h is: %17.15f \n\n", M_PI);
        pi_serial(num_sub_intervals);
        printf("Difference between computed pi and math.h M_PI = %17.15f\n\n",
               fabs(pi_tot - M_PI));
        printf("Time to compute = %g seconds\n\n", time_diff);
    }
    //Line is for end of parallel regions of the code.
    MPI_Finalize();
    return EXIT_SUCCESS;
}

double pi_serial(int num_iter)
{
    // The following variables are declared.
    double pi; // is for the calculated value of pi
    double rect_width; // is for the width of each rectangle
    double rect_left; // is for the left-most x-coordinate of each rectangle
    double rect_height; // is for the height of each rectangle
    double agg_area; // is for the aggregate area
    long long int N_Rects;
    int i; // is for looping over the rectangles

    // The following commands initialize the algorithm.
    N_Rects = num_iter; // sets the # of rectangles to its default
    rect_width = 1.0 / N_Rects; // Uses the # of rectangles to calculate the
                                // width of each rectangle

    // The following commands run the algorithm.
    agg_area = 0.0; // initializes the aggregate area
    rect_left = 0.0; // starts things off at the origin
    for (i=0; i<N_Rects; i++) { // begins a loop over each rectangle
        rect_left += rect_width; // finds the next rectangle's left-side x-coord
        rect_height = sqrt(1-rect_left*rect_left); // calculates the rectangle's
                                                   // height
        agg_area += rect_width*rect_height; // calculates the rectangle's area
                                            // and adds it to the aggregate
                                            // area.
    }
    pi = 4*agg_area; // calculates the final value of pi

    printf("Computed pi value in serial is: %.12f\n", pi); // prints the final value of pi to 12 decimal places,
                           // an arbitrary amount

    return pi; 
}
