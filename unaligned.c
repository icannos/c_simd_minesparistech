#include <stdio.h>
#include <stdlib.h>

#include <immintrin.h>
#include <math.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#define VECT 1
#define SCALAR 0

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



// Classical norm function
float norm(float *U, long N) {
    float d=0;
    for (long i = 0; i < N; i++)
        d += sqrtf(fabsf(U[i]));

    return d;
}


float vect_norm(float *U, unsigned int N) {
    // ptr on the array to perform the sum on
    __m256* u_v = (__m256*) U;

    // Accumulator to store 8 partial sums
    __m256 acc = _mm256_set1_ps(0.0f);

    // Used later to sum horitally over the vector
    float *acc_fptr = (float *) &acc;

    // Sign mask to take abs value
    __m256 sign_mask = _mm256_set1_ps(-0.f);

    // We use vector operation to compute the square root and the absolute value
    // Then we add the vector to the accumulator using vector add
    // Doing so we gain a x8 in time to compute the sum
    for (unsigned int i = 0; i < N / 8; i++)
        acc = _mm256_add_ps(acc, _mm256_sqrt_ps(_mm256_andnot_ps(sign_mask, u_v[i])));

    // We only have to sum the 8 float in the acc vector
    float result = 0;
    for (unsigned int i = 0; i < 8; i++)
        result += acc_fptr[i];

    return result;
}

// to be passed to each thread
typedef struct {
    // begining of the array to consider
    float *begin;
    // Where to store the result of each thread
    float *result;
    // size of the considered array
    long size;

} threadarg_t;

// routine used to encapsulate the call to the norm function in each thread
void norm_routine(threadarg_t* args)
{
    // We compute the norm using the given norm function
    // We store the result at the requested adress
    *args->result = vect_norm(args->begin, args->size);

    // We terminate the thread
    pthread_exit(NULL);
}

float normPar(float *U, long N, unsigned char mode, unsigned  int nb_threads) {
    // pointer to the norm function to use


    // if more than one thread (one thread is actually handle in the main thread / process
    if (mode == VECT) {
        // We begin by initializing the argument for each thread

        long elt_per_thread = (long) (N / nb_threads);

        // In case of non aligned data
        long rem = N % nb_threads;

        threadarg_t *args = (threadarg_t *) aligned_alloc(CACHE_LINE_SIZE, sizeof(threadarg_t) * nb_threads);

        pthread_t *pool = (pthread_t *) aligned_alloc(CACHE_LINE_SIZE, sizeof(pthread_t) *  (nb_threads-1));
        float *results = (float *) aligned_alloc(CACHE_LINE_SIZE, sizeof(float) * nb_threads);

        int errcode = 0;

        for (unsigned int i = 1; i < nb_threads; i++) {
            // We construct the argument for each thread
            args[i].begin = &(U[i * elt_per_thread]);
            args[i].size = elt_per_thread;
            args[i].result = results+i;

            // We create the thread
            errcode += (int) pthread_create(&pool[i], NULL, (void* (*)(void*)) norm_routine, &args[i]);

        }

        if (errcode != 0) {
            printf("Something went wrong with thread creation");
            exit(1);
        }

        // Computations in the main thread
        // We direclty store it in our result variable
        float r = vect_norm(U, elt_per_thread);

        errcode = 0;

        for (unsigned i = 1; i < nb_threads; i++) {

            errcode += pthread_join(pool[i], NULL);

            // When the threads end, we retrieve their result and add it into the result variable
            r += results[i];

        }

        // Check if the joins succeded
        if (errcode != 0) {
            printf("Something went wront with thread join");
            exit(1);
        }

        // Free our memory
        free(pool);
        free(args);
        free(results);

        return r;
    }
    else {
        // Single thread: just call the wanted function on the array
        float result = norm(U, N);

        // Return the result
        return result;

    }
}


int main(int argc, char *argv[]) {

    // Check for arguments
    if(argc < 3) {
        printf("Not enough arguments. 2 are required");
        exit(1);
    }

    // init random seed
    srand((unsigned int)time(NULL));

    // Get number of elements
    unsigned N = (unsigned int) atoi(argv[1]);

    // Get number of threads
    unsigned int nb_thread = (unsigned int) atoi(argv[2]);

    // We allocate our array
    // We align our array: it has 2 purposes: first it optimizes the cache
    // and it guarantees us that the data are well aligned for the vectorial instructions
    // since my cache line is 64
    float* U = (float*) aligned_alloc(CACHE_LINE_SIZE, sizeof(float)*N);

    // Initialization
    for (long i = 0; i < N; i++)
        U[i] = ((float)rand()/(float)(RAND_MAX));
        //U[i] = 1.0f; // To easily check the correctness of the output

    // Use to store the result
    float result;

    // =============================================================== \\
    // Test of the classical method on a single thread

    struct timespec begining_classic;
    clock_gettime(CLOCK_REALTIME, &begining_classic);

    result = normPar(U, N, SCALAR, 1);

    struct timespec end_classic;
    clock_gettime(CLOCK_REALTIME, &end_classic);

    printf("%e\n", result);

    // =============================================================== \\
    // Test of the vectoriel method multithreaded
    struct timespec begining_vect_thread;
    clock_gettime(CLOCK_REALTIME, &begining_vect_thread);

    result = normPar(U, N, VECT, nb_thread);

    struct timespec end_vect_thread;
    clock_gettime(CLOCK_REALTIME, &end_vect_thread);


    printf("%e\n", result);

    printf("%e\n", result);

    // =============================================================== \\
    // Execution time comparison

    struct timespec classic = diff(begining_classic, end_classic);
    struct timespec vect = diff(begining_vect_thread, end_vect_thread);

    double d1 = (double) (classic.tv_sec * 1000000000l + classic.tv_nsec) * 1E-9;
    double d2 = (double) (vect.tv_sec * 1000000000l + vect.tv_nsec) * 1E-9;


    printf("Usual scalar norm, 1 thread: %e\n", d1);
    printf("Vectorized norm, %d thread: %e\n", nb_thread, d2);

    printf("Speedup x%0.1f\n", d1 / d2);
    // free our memory
    free(U);


    return 0;
}
