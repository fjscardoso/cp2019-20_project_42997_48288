#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>

#include "unit.h"
#include "debug.h"

#define TYPE double


////////////////////////////////////////////////////////////////////////////////////////
/// Get wall clock time as a double
/// You may replace this with opm_get_wtime()
double wctime () {
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return tv.tv_sec + 1E-6 * tv.tv_usec;
}



int main(int argc, char* argv[]) {
    int i, N;
    

    int c;
    while ((c = getopt (argc, argv, "d")) != -1)
    switch (c) {
        case 'd':
            debug = 1; break;
        default:
            printf("Invalid option\n");
            abort ();
    }
    argc -= optind;
    argv += optind;
    
    if (argc != 1) {
        printf("Usage: ./example N\n");
        return -1;
    }

    srand48(time(NULL));
    srand48(time(NULL));

    N = atol(argv[0]);


    printf ("Initializing SRC array\n");
    TYPE *src = malloc (sizeof(*src) * N);
    for (i = 0; i < N; i++)
        src[i] = drand48();
    printf ("Done!\n");
    
    printDouble (src, N, "SRC");
    if (debug)
        printf ("\n\n");

    for (int i = 0;  i < nTestFunction;  i++) {
        double start = wctime();
        testFunction[i] (src, N, sizeof(*src));
        double end = wctime();
        printf ("%s:\t%6.3lf seconds\n", testNames[i], end-start);
        if (debug)
            printf ("\n\n");
    }

    return 0;
}
