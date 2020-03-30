#include <string.h>
#include <assert.h>
#include "patterns.h"

void map (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2)) {
    /* To be implemented */
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);
    for (int i = 0;  i < nJob;  i++) {
        worker (&dest[i * sizeJob], &src[i * sizeJob]);
    }
}

void reduce (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    /* To be implemented */
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);
    if (nJob > 0) {
        memcpy (&dest[0], &src[0], sizeJob);
        for (int i = 1;  i < nJob;  i++)
            worker (&dest[0], &dest[0], &src[i * sizeJob]);
    }
}

void scan (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3)) {
    /* To be implemented */
    assert (dest != NULL);
    assert (src != NULL);
    assert (worker != NULL);
    if (nJob > 1) {
        memcpy (&dest[0], &src[0], sizeJob);
        for (int i = 1;  i < nJob;  i++)
            worker (&dest[i * sizeJob], &dest[(i-1) * sizeJob], &src[i * sizeJob]);
    }
}

int pack (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter) {
    /* To be implemented */
    int pos = 0;
    for (int i=0; i < nJob; i++) {
        if (filter[i]) {
            memcpy (&dest[pos * sizeJob], &src[i * sizeJob], sizeJob);
            pos++;
        }
    }
    return pos;
}

void gather (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter, int nFilter) {
    /* To be implemented */
    for (int i=0; i < nFilter; i++) {
        memcpy (&dest[i * sizeJob], &src[filter[i] * sizeJob], sizeJob);
    }
}

void scatter (void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter) {
    /* To be implemented */
    for (int i=0; i < nJob; i++) {
        memcpy (&dest[filter[i] * sizeJob], &src[i * sizeJob], sizeJob);
    }
}

void pipeline (void *dest, void *src, size_t nJob, size_t sizeJob, void (*workerList[])(void *v1, const void *v2), size_t nWorkers) {
    /* To be implemented */
    for (int i=0; i < nJob; i++) {
        memcpy (&dest[i * sizeJob], &src[i * sizeJob], sizeJob);
        for (int j = 0;  j < nWorkers;  j++)
            workerList[j] (&dest[i * sizeJob], &dest[i * sizeJob]);
    }
}

void farm (void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2), size_t nWorkers) {
    /* To be implemented */
    map (dest, src, nJob, sizeJob, worker);  // it provides the right result, but is a very very vey bad implementation…
}
