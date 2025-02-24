// project3.c
// Group 3 - Alex Boyce, Noah Rodal, Saul Rodriguez-Tapia


//Write an MPI parallel program to determine, for all integers less than 1, 000, 000, the number of times that
//two consecutive odd integers are both prime.

//This can be solved using the Sieve of Eratosthenes
//a proper solution would have 8169 twin primes under 1 million

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>

#define LIMIT 1000000 //all integers less than 1mil

//prime lookup table
bool prime[LIMIT + 1];

//sieve of Eratosthenes to mark primes
void sieve()
{
    for (int i = 0; i <= LIMIT; i++) prime[i] = true;
    prime[0] = prime[1] = false;

    for (int i = 2; i * i <= LIMIT; i++)
    {
        if (prime[i])
        {
            for (int j = i * i; j <= LIMIT; j += i)
            {
                prime[j] = false;
            }
        }
    }
}

int main(int argc, char** argv)
{
    int rank, size, local_count = 0, global_count = 0;

    //initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //start the sieve if this is the first process (rank 0)
    if (rank == 0)
    {
        sieve();  //start initializes sieve
    }

    //mpi broadcast the prime lookup table
    MPI_Bcast(prime, LIMIT + 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);

    //define the workload for each process
    int start = 3 + (rank * 2);  //start from the first odd number for this rank
    int step = 2 * size;         //step by 2 * number of processes to ensure even workload

    //count twin primes in assigned range
    for (int i = start; i < LIMIT - 2; i += step)
    {
        if (prime[i] && prime[i + 2])
        {
            local_count++;
        }
    }

    //collect and sum all counts from all processes
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    //print the total count of twin primes once all processes have finished
    if (rank == 0)
    {
        printf("Total twin primes below %d: %d\n", LIMIT, global_count);
    }

    //finalize MPI
    MPI_Finalize();
    return 0;
}