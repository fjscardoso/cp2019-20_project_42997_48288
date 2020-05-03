#include <string.h>
#include <assert.h>
#include "patterns.h"
#include <omp.h>

#include <stdio.h>

#define TYPE double

//======================================================================================================================
// MAP
//======================================================================================================================
//SEQUENTIAL
void mapSequential(void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2))
{
    /* To be implemented */
    assert(dest != NULL);
    assert(src != NULL);
    assert(worker != NULL);
    char *d = dest;
    char *s = src;
    for (int i = 0; i < nJob; i++)
    {
        worker(&d[i * sizeJob], &s[i * sizeJob]);
    }
}

//PARALLEL
void map(void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2))
{
    /* To be implemented */
    //int nthreads, tid;
    assert(dest != NULL);
    assert(src != NULL);
    assert(worker != NULL);
    char *d = dest;
    char *s = src;
#pragma omp parallel for num_threads(8)
    for (int i = 0; i < nJob; i++)
    {
        //tid = omp_get_thread_num();
        //printf("%d\n", tid);
        worker(&d[i * sizeJob], &s[i * sizeJob]);
    }
}

//======================================================================================================================
// REDUCE
//======================================================================================================================
void reduceSequential(void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3))
{

    assert(dest != NULL);
    assert(src != NULL);
    assert(worker != NULL);
    char *d = dest;
    char *s = src;
    if (nJob > 0)
    {
        memcpy(&d[0], &s[0], sizeJob);
        for (int i = 1; i < nJob; i++)
            worker(&d[0], &d[0], &s[i * sizeJob]);
    }
}

void reduce(void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3))
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(worker != NULL);

    int maxWorkers = omp_get_max_threads();
    int jobsPerWorker = nJob / maxWorkers;
    int jobsRemainder = nJob % maxWorkers;

    void *auxArray = malloc(sizeJob * nJob);

    if (nJob > 1)
    {

#pragma omp parallel for
        for (int k = 0; k < maxWorkers; k++)
        {
            int startJob = k * jobsPerWorker;
            int endJob = startJob + jobsPerWorker;
            endJob = k == maxWorkers - 1 ? (endJob + jobsRemainder) : endJob;

            for (int j = startJob; j < endJob; j++)
            {
                void *a = auxArray + startJob * sizeJob;
                void *b = src + j * sizeJob;
                worker(a, a, b);
            }
        }

        for (int i = 1; i < maxWorkers; i++)
        {
            void *a = auxArray;
            void *b = auxArray + i * sizeJob * jobsPerWorker;
            worker(a, a, b);
        }

        memcpy(dest, auxArray, sizeJob);
        free(auxArray);
    }
}

//======================================================================================================================
// SCAN
//======================================================================================================================
void scan(void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3))
{
    /* To be implemented */
    assert(dest != NULL);
    assert(src != NULL);
    assert(worker != NULL);
    char *d = dest;
    char *s = src;
    if (nJob > 1)
    {
        memcpy(&d[0], &s[0], sizeJob);
        for (int i = 1; i < nJob; i++)
            worker(&d[i * sizeJob], &d[(i - 1) * sizeJob], &s[i * sizeJob]);
    }
}

//======================================================================================================================
// PACK
//======================================================================================================================
int pack(void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter)
{
    /* To be implemented */
    assert(dest != NULL);
    assert(src != NULL);
    assert(filter != NULL);
    assert(nJob >= 0);
    assert(sizeJob > 0);
    char *d = dest;
    char *s = src;
    int pos = 0;
    for (int i = 0; i < nJob; i++)
    {
        if (filter[i])
        {
            memcpy(&d[pos * sizeJob], &s[i * sizeJob], sizeJob);
            pos++;
        }
    }
    return pos;
}

//======================================================================================================================
// GATHER
//======================================================================================================================
void gatherSequential(void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter, int nFilter)
{
    /* To be implemented */
    assert(dest != NULL);
    assert(src != NULL);
    assert(filter != NULL);
    assert(nJob >= 0);
    assert(sizeJob > 0);
    assert(nFilter >= 0);
    char *d = dest;
    char *s = src;
    for (int i = 0; i < nFilter; i++)
    {
        assert(filter[i] < nJob);
        memcpy(&d[i * sizeJob], &s[filter[i] * sizeJob], sizeJob);
    }
}

void gather(void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter, int nFilter)
{
    /* To be implemented */
    assert(dest != NULL);
    assert(src != NULL);
    assert(filter != NULL);
    assert(nJob >= 0);
    assert(sizeJob > 0);
    assert(nFilter >= 0);
    char *d = dest;
    char *s = src;
#pragma omp parallel for num_threads(4)
    for (int i = 0; i < nFilter; i++)
    {
        assert(filter[i] < nJob);
        memcpy(&d[i * sizeJob], &s[filter[i] * sizeJob], sizeJob);
    }
}

//======================================================================================================================
// SCATTER
//======================================================================================================================
void scatterSequential(void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter)
{
    /* To be implemented */
    assert(dest != NULL);
    assert(src != NULL);
    assert(filter != NULL);
    assert(nJob >= 0);
    assert(sizeJob > 0);
    char *d = dest;
    char *s = src;
    for (int i = 0; i < nJob; i++)
    {
        assert(filter[i] < nJob);
        memcpy(&d[filter[i] * sizeJob], &s[i * sizeJob], sizeJob);
    }
}

void scatter(void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter)
{
    /* To be implemented */
    assert(dest != NULL);
    assert(src != NULL);
    assert(filter != NULL);
    assert(nJob >= 0);
    assert(sizeJob > 0);
    char *d = dest;
    char *s = src;

    int *auxFilter = malloc(sizeof(int) * nJob);
    memset(auxFilter, -1, sizeof(int) * nJob);

#pragma omp parallel for
    for (int i = 0; i < nJob; i++)
    {
        assert(filter[i] < nJob);
        //printf("auxFilter[%d]:%d, auxFilter[filter[%d]]:%d\n", i, auxFilter[i], i, auxFilter[filter[i]]);
        if (auxFilter[i] == -1 || i > auxFilter[filter[i]])
        {
            memcpy(&d[filter[i] * sizeJob], &s[i * sizeJob], sizeJob);
            auxFilter[filter[i]] = i;
        }
    }
}

//======================================================================================================================
// PIPELINE
//======================================================================================================================
void pipeline(void *dest, void *src, size_t nJob, size_t sizeJob, void (*workerList[])(void *v1, const void *v2), size_t nWorkers)
{
    /* To be implemented */
    assert(dest != NULL);
    assert(src != NULL);
    assert(workerList != NULL);
    assert(nJob >= 0);
    assert(sizeJob > 0);
    char *d = dest;
    char *s = src;
    for (int i = 0; i < nJob; i++)
    {
        memcpy(&d[i * sizeJob], &s[i * sizeJob], sizeJob);
        for (int j = 0; j < nWorkers; j++)
        {
            assert(workerList[j] != NULL);
            workerList[j](&d[i * sizeJob], &d[i * sizeJob]);
        }
    }
}

//======================================================================================================================
// FARM
//======================================================================================================================
void farm(void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2), size_t nWorkers)
{
    /* To be implemented */
    map(dest, src, nJob, sizeJob, worker); // it provides the right result, but is a very very vey bad implementation…
}
