//
// Created by maxime on 11/12/2020.
//

#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <time.h>

// On my machine a cache line is 64 bytes long
#define CACHE_LINE_SIZE 64

struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;

    if (end.tv_nsec-start.tv_nsec<0)
    {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    }
    else
    {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

float norm(float *U, unsigned int N) {
    // We split the computations in two blocks because we keep adding small numbers to a large float
    // thus leading to add only 0 each time
    float d1 = 0.0f;
    for (unsigned int i = 0; i < N; i++) {
        d1 += sqrtf(fabsf(U[i]));
    }

    return d1;
}

float normPar(float *U, unsigned int N, unsigned int nb_threads) {
        // Single thread: just call the wanted function on the array
        float result = norm(U, N);

        // Return the result
        return result;

}

int main(int argc, char *argv[]) {

    // Check for arguments
    if (argc < 2) {
        printf("Not enough arguments. 2 are required");
        exit(1);
    }

    // init random seed
    srand((unsigned int) time(NULL));

    // Get number of elements
    unsigned int N = (unsigned int) atoi(argv[1]);

    // We allocate our array
    // We align our array: it has 2 purposes: first it optimizes the cache
    // and it guarantees us that the data are well aligned for the vectorial instructions
    // since my cache line is 64
    // It also ensures that parallel words are well aligned
    float *U = (float *) malloc(sizeof(float) * N);

    // Initialization
    for (unsigned int i = 0; i < N; i++)
        U[i] = ((float) rand() / (float) (RAND_MAX));

    // Use to store the result
    float result;

    // =============================================================== \\
    // Test of the classical method on a single thread

    struct timespec begining_classic;
    clock_gettime(CLOCK_REALTIME, &begining_classic);

    result = normPar(U, N, 1);

    struct timespec end_classic;
    clock_gettime(CLOCK_REALTIME, &end_classic);

    // printf("%e\n", result);

    // =============================================================== \\
    // Execution time comparison
    struct timespec classic = diff(begining_classic, end_classic);


    double t1 = (double) (classic.tv_sec * 1000000000l + classic.tv_nsec) * 1E-9;


    printf("Usual scalar norm, 1 thread: %e\n", t1);

    // free our memory
    free(U);


    return 0;
}
