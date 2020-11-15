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

// type of a norm function
// Used to pass the function to use to each thread through a structure
typedef float (*normfn_t)(float *, long);

__m256 abs_ps(__m256 x) {
    // In order to get the absolute value on only need to flip the first bit to 0
    // We use a mask with a bit to 1 on each first bit of each component of the vector
    // Then we do a logical not and.
    __m256 sign_mask = _mm256_set1_ps(-0.f); // -0.f = 1 << 31
    return _mm256_andnot_ps(sign_mask, x);
}

// Classical norm function
float norm(float *U, long N) {
    float d = 0;
    for (long i = 0; i < N; i++)
        d += sqrtf(fabsf(U[i]));
    return d;
}


float vect_norm(float *U, long N) {
    // ptr on the array to perform the sum on
    __m256 *U_vect = (__m256 *) U;

    // Accumulator to store 8 partial sums
    __m256 acc = _mm256_set1_ps(0.0f);

    // Used later to sum horitally over the vector
    float *acc_fptr = (float *) &acc;

    // We use vector operation to compute the square root and the absolute value
    // Then we add the vector to the accumulator using vector add
    // Doing so we gain a x4 in time to compute the sum
    for (long i = 0; i < N / 8; i++)
        acc = _mm256_add_ps(acc, _mm256_sqrt_ps(abs_ps(U_vect[i])));

    // We only have to sum the 8 float in the acc vector
    float result = 0;
    for (long i = 0; i < 8ll; i++)
        result += acc_fptr[i];

    return result;
}

// to be passed to each thread
typedef struct {
    // A pointer to the norm function of type
    // float (*)(float*, unsigned int)
    normfn_t norm_fn;
    // begining of the array to consider
    float *begin;
    // Where to store the result of each thread
    float *result;
    // size of the considered array
    long size;

} array_t;

// routine used to encapsulate the call to the norm function in each thread
void norm_routine(array_t* args)
{
    // We compute the norm using the given norm function
    // We store the result at the requested adress
    *(args->result) = args->norm_fn(args->begin, args->size);

    // We terminate the thread
    pthread_exit(NULL);
}

float normPar(float *U, long N, unsigned char mode, unsigned  int nb_threads) {
    // pointer to the norm function to use
    normfn_t norm_fn;

    // depends on the mode
    switch (mode) {
        case SCALAR:
            norm_fn = &norm;
            break;
        case VECT:
            norm_fn = &vect_norm;
            break;
        default: // by default we use the scalar function
            norm_fn = &norm;
            break;
    }


    // if more than one thread (one thread is actually handle in the main thread / process
    if (nb_threads > 1) {
        // We begin by initializing the argument for each thread

        long elt_per_thread = (long) (N / nb_threads);
        long rem = N % nb_threads;

        //printf("%d\n", elt_per_thread);

        array_t *args = (array_t *) aligned_alloc(CACHE_LINE_SIZE, sizeof(array_t) * nb_threads);

        pthread_t *pool = (pthread_t *) aligned_alloc(CACHE_LINE_SIZE, sizeof(pthread_t) *  (nb_threads-1));
        float *results = (float *) aligned_alloc(CACHE_LINE_SIZE, sizeof(float) * nb_threads);

        long errcode = 0;
        for (long i = 1; i < nb_threads; i++) {
            // We construct the argument for each thread
            args[i].begin = &U[i * elt_per_thread];
            args[i].size = elt_per_thread;
            args[i].norm_fn = norm_fn;
            args[i].result = &results[i];
            // We create the thread
            errcode += (long) pthread_create(&pool[i], NULL, (void* (*)(void*)) norm_routine, &args[i]);
        }

        if (errcode != 0) {
            printf("Something went wront with thread creation");
            exit(1);
        }

        args[0].begin = U;
        args[0].size = elt_per_thread;
        args[0].norm_fn = norm_fn;
        args[0].result = &results[0];


        float r = norm_fn(U, elt_per_thread);

        errcode = 0;

        for (long i = 1; i < nb_threads; i++) {
            errcode += pthread_join(pool[i], NULL);
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
        float result = norm_fn(U, N);

        // Return the result
        return result;

    }
}


int main(int argc, char *argv[]) {
    if(argc < 3) {
        printf("Not enough arguments. 2 are required");
        exit(1);
    }

    srand((long)time(NULL));

    long N = (long) atol(argv[1]);
    unsigned int nb_thread = (long) atol(argv[2]);

    float* U = (float*) aligned_alloc(CACHE_LINE_SIZE, sizeof(float)*N);

    for (long i = 0; i < N; i++)
    {
     U[i] = ((float)rand()/(float)(RAND_MAX));
        //U[i] = 0.0000001f;
    }

    float result;

    clock_t begining_classic = clock();
    result = normPar(U, N, SCALAR, 1);
    //result = norm(U, N);
    clock_t end_classic = clock();

    printf("%e\n", result);

    clock_t begining_classic_thread = clock();
    result = normPar(U, N, SCALAR, nb_thread);
    clock_t end_classic_thread = clock();
    printf("%e\n", result);

    clock_t begining_vect = clock();
    result = normPar(U, N, VECT, 1);
    clock_t end_vect = clock();
    printf("%e\n", result);

    clock_t begining_vect_thread = clock();
    result = normPar(U, N, VECT, nb_thread);
    clock_t end_vect_thread = clock();
    printf("%e\n", result);


    printf("Usual scalar norm, 1 thread: %e\n", (double) ((double) (end_classic-begining_classic)/CLOCKS_PER_SEC));
    printf("Usual scalar norm, %d thread: %e\n", nb_thread, (double) ((double)(end_classic_thread-begining_classic_thread)/CLOCKS_PER_SEC));
    printf("Vectorized norm, 1 thread: %e\n", (double) ((double)(end_vect - begining_vect)/CLOCKS_PER_SEC));
    printf("Vectorized norm, %d thread: %e\n",  nb_thread, (double) ((double)(end_vect_thread - begining_vect_thread)/CLOCKS_PER_SEC));

    free(U);


    return 0;
}
