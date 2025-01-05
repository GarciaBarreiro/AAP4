#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <sys/time.h>

#define INIT_TIME(prev, init) \
    gettimeofday(&prev, NULL); \
    gettimeofday(&init, NULL);

// remove overhead created by call to gettimeofday
#define GET_TIME(prev, init, final, res) \
    gettimeofday(&final, NULL); \
    res = (final.tv_sec-init.tv_sec+(final.tv_usec-init.tv_usec)/1.e6) - \
          (init.tv_sec-prev.tv_sec+(init.tv_usec-prev.tv_usec)/1.e6);

int main(int argc, char *argv[]) {
    struct timeval t_prev, t_init, t_final;
    double t_seq, t_par;

    unsigned N = (argc > 1) ? atoi(argv[1]) : 1000;
    unsigned numElem = N * N;
    size_t size = N * N * sizeof(float);

    float *A  = (float *)malloc(size);
    float *B  = (float *)malloc(size);
    float *Cs = (float *)malloc(size);  // result sequential
    float *Cp = (float *)malloc(size);  // result parallel

    if (!A || !B || !Cs || !Cp) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    for (unsigned i = 0; i < numElem; i++) {
        A[i] = rand() / (float)RAND_MAX;
        B[i] = rand() / (float)RAND_MAX;
    }

#ifdef DEBUG
    // seq
    INIT_TIME(t_prev, t_init);
    for (unsigned i = 0; i < N; i++) {
        for (unsigned j = 0; j < N; j++) {
            float sum = 0;
            for (unsigned k = 0; k < N; k++) {
                sum += A[i * N + k] * B[k * N + j];
            }
            Cs[i * N + j] = sum;
        }
    }
    GET_TIME(t_prev, t_init, t_final, t_seq);
#endif

    // par
    INIT_TIME(t_prev, t_init);
    #pragma omp parallel for collapse(2)
    for (unsigned i = 0; i < N; i++) {
        for (unsigned j = 0; j < N; j++) {
            float sum = 0;
            for (unsigned k = 0; k < N; k++) {
                sum += A[i * N + k] * B[k * N + j];
            }
            Cp[i * N + j] = sum;
        }
    }
    GET_TIME(t_prev, t_init, t_final, t_par);

#ifdef DEBUG
    // check errors
    for (unsigned i = 0; i < numElem; i++) {
        if (fabs(Cs[i] - Cp[i]) > 1e-3) {
            fprintf(stderr, "error at index %d: %f != %f\n", i, Cs[i], Cp[i]);
            return 1;
        }
    }
#endif

    free(A);
    free(B);
    free(Cs);
    free(Cp);

#ifdef DEBUG
    printf("Sequential time: %f\n", t_seq);
#endif
    printf("Parallel time: %f\n", t_par);

#ifdef DEBUG
    FILE *f = fopen((argc > 2) ? argv[2] : "output.txt", "a");
    fprintf(f, "%d,%d,%f,%f\n", N, omp_get_max_threads(), t_seq, t_par);
    fclose(f);
#else
    FILE *f = fopen((argc > 2) ? argv[2] : "output.txt", "a");
    fprintf(f, "%d,%d,%f\n", N, omp_get_max_threads(), t_par);
    fclose(f);
#endif

    return 0;
}
