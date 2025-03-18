// project4.c
// Group 3 - Alex Boyce, Noah Rodal, Saul Rodriguez-Tapia



#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdbool.h>
#include <time.h>

#define LIMIT 1000000 // all integers less than 1mil
#define RUNS 5 // Number of benchmark runs

bool prime[LIMIT + 1]; // Prime lookup table

typedef struct {
    double time;
} BenchmarkResult;

// Sieve of Eratosthenes to mark primes
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
    BenchmarkResult results[RUNS];

    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for (int run = 0; run < RUNS; run++) {
        double start_time = MPI_Wtime();

        // Process 0 initializes the sieve
        if (rank == 0) {
            sieve();
            // Start pipeline send
            MPI_Send(prime, LIMIT + 1, MPI_C_BOOL, 1, 0, MPI_COMM_WORLD);
        }
        else {
            // Other processes receive and forward
            MPI_Recv(prime, LIMIT + 1, MPI_C_BOOL, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (rank < size - 1) {
                MPI_Send(prime, LIMIT + 1, MPI_C_BOOL, rank + 1, 0, MPI_COMM_WORLD);
            }
        }

        // Define workload for each process
        int start = 3 + (rank * 2);
        int step = 2 * size;

        // Count twin primes in assigned range
        local_count = 0;
        for (int i = start; i < LIMIT - 2; i += step)
        {
            if (prime[i] && prime[i + 2])
            {
                local_count++;
            }
        }

        // Collect and sum all counts from all processes
        MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        double end_time = MPI_Wtime();
        results[run].time = end_time - start_time;
    }

    // Compute average execution time
    if (rank == 0) {
        double avg_time = 0;
        for (int i = 0; i < RUNS; i++) {
            avg_time += results[i].time;
        }
        avg_time /= RUNS;
        printf("Total twin primes below %d: %d\n", LIMIT, global_count);
        printf("Average execution time over %d runs: %f seconds\n", RUNS, avg_time);
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}
