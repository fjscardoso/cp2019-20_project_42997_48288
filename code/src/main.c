#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>

#include "unit.h"
#include "debug.h"
#include <omp.h>

#define TYPE double

////////////////////////////////////////////////////////////////////////////////////////
/// Get wall clock time as a double
/// You may replace this with opm_get_wtime()
double wctime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + 1E-6 * tv.tv_usec;
}

int main(int argc, char *argv[])
{
    int i, N, threads;

    //Code to check input from command line
    int c;
    while ((c = getopt(argc, argv, "d")) != -1)
        switch (c)
        {
        case 'd':
            debug = 1;
            break;
        default:
            printf("Invalid option\n");
            abort();
        }
    argc -= optind;
    argv += optind;

    // if (argc != 1)
    // {
    //     printf("Usage: ./example N\n");
    //     return -1;
    // }
    if (argc == 2)
    {
        threads = atoi(argv[0]);
        //printf("THREADS: %i\n", threads);
    }

    //Initialization functions for random numbers
    srand48(time(NULL));
    srand48(time(NULL));

    //Converts String to long
    N = atol(argv[1]);

    //Malloc memory for [] src
    //printf("Initializing SRC array\n");
    TYPE *src = malloc(sizeof(*src) * N);

    //For to fill [] src with longs
    for (i = 0; i < N; i++)
        src[i] = drand48();
    //printf("Done!\n");

    //printDouble(src, N, "SRC");

    if (debug)
        printf("\n\n");

    //For to iterate all test functions and print time wasted on computation
    for (int i = 0; i < nTestFunction; i++)
    {
        double start = omp_get_wtime();
        testFunction[i](src, N, sizeof(*src));
        double end = omp_get_wtime();
        printf("%d,%s,%6.3lf,%d\n", N, testNames[i], end - start, threads);
        if (debug)
            printf("\n\n");
    }

    free(src);
    return 0;
}
