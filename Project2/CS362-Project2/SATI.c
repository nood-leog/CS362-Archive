/*
 *   Circuit Satisfiability
 *
 *   This version of the program prints the total number of
 *   solutions and the execution time.
 *
 *   Project 1
 */

 //Group 3 - Alex Boyce, Noah Rodal, Saul Rodriguez-Tapia


 //Benchmark the second circuit satisfiability program for 1, ..., 8 processors with printing disabled. For each
 //number of processors, determine the average execution time after five runs. Summarize and interpret the
 //observed results.

#include "mpi.h"
#include <stdio.h>

int main (int argc, char *argv[]) 
{
   int count;            /* Solutions found by this proc */
   int global_count;     /* Total number of solutions */
   int i;             
   int id;               /* Process rank */
   int p;                /* Number of processes */

   int num_trials = 5;  //number of trials to run
   int num_processors = 8; //number of processors to test


   int check_circuit(int, int);
 
   double elapsed_time;  /* Time to find, count solutions */
   double total_time = 0.0; //total time
   double average_time = 0.0; //average time


   MPI_Init(&argc, &argv); //start MPI 
   MPI_Comm_rank(MPI_COMM_WORLD, &id); //get the rank of the current process
   MPI_Comm_size(MPI_COMM_WORLD, &p); //get the number of processes

   //PROCESSORS
   //iterate from 8 processors down to 1
   for (int num_processors = 8; num_processors >= 1; num_processors--) 
   {
	   total_time = 0.0; //set total time to 0 for each processor count

       //TRIALS
	   //run the program 5 times as num_trials is set to 5
       for (int trial = 0; trial < num_trials; trial++) 
       {    MPI_Barrier(MPI_COMM_WORLD);
            elapsed_time = -MPI_Wtime();

           count = 0;
           for (i = id; i < 65536; i += num_processors)
               count += check_circuit(id, i);

           MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
           elapsed_time += MPI_Wtime();
           total_time += elapsed_time;

           MPI_Barrier(MPI_COMM_WORLD);
       }

	   //get the average time
        average_time = total_time / num_trials;

       if (!id) 
       {
           printf("Processors: %d, Average Execution Time: %8.6f seconds\n", num_processors, average_time);
           printf("Solutions Found: %d\n", global_count);
           fflush(stdout);
       }
   }

   printf("Total time: %f\n", total_time);

   MPI_Finalize(); //end MPI
   return 0;
}

/* Return 1 if 'i'th bit of 'n' is 1; 0 otherwise */
#define EXTRACT_BIT(n,i) ((n&(1<<i))?1:0)

int check_circuit (int id, int z) {
   int v[16];        /* Each element is a bit of z */
   int i;

   for (i = 0; i < 16; i++) v[i] = EXTRACT_BIT(z,i);
   if ((v[0] || v[1]) && (!v[1] || !v[3]) && (v[2] || v[3])
      && (!v[3] || !v[4]) && (v[4] || !v[5])
      && (v[5] || !v[6]) && (v[5] || v[6])
      && (v[6] || !v[15]) && (v[7] || !v[8])
      && (!v[7] || !v[13]) && (v[8] || v[9])
      && (v[8] || !v[9]) && (!v[9] || !v[10])
      && (v[9] || v[11]) && (v[10] || v[11])
      && (v[12] || v[13]) && (v[13] || !v[14])
      && (v[14] || v[15])) {
      
       /*
      printf ("%d) %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d\n", id,
         v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8],v[9],
         v[10],v[11],v[12],v[13],v[14],v[15]);
      */

      fflush (stdout);
      return 1;
   } else return 0;
}

