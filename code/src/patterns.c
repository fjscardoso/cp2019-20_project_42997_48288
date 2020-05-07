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
void scanSequential(void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3))
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



void scan(void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3))
{
    /* To be implemented */
    assert(dest != NULL);
    assert(src != NULL);
    assert(worker != NULL);
    //char *d = dest;
    //char *s = src;

    struct node* root = malloc(sizeof(struct node));
    root->right = malloc(sizeof(struct node));
    root->left = malloc(sizeof(struct node));
    root->sum = malloc(sizeJob);
    root->fromleft = malloc(sizeJob);

    root->fromleft = 0;

    #pragma omp parallel num_threads(4)
    {
    #pragma omp single
        upPass(root, src, 0, nJob, sizeJob, worker);

    #pragma omp single
    {
        #pragma omp task
            downPass(src, dest, -1, root, root->right, sizeJob, worker);

        #pragma omp task
            downPass(src, dest, 1, root, root->left, sizeJob, worker);

    }
}
    //#pragma omp taskwait



/**    if (nJob > 1)
    {
        memcpy(&d[0], &s[0], sizeJob);
        for (int i = 1; i < nJob; i++)
            worker(&d[i * sizeJob], &d[(i - 1) * sizeJob], &s[i * sizeJob]);
    }
    */
}

void upPass(struct node* root, void *src, size_t lo, size_t hi, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3))
{

    root->right = malloc(sizeof(struct node));
    root->left = malloc(sizeof(struct node));
    root->index = -1;
    root->sum = malloc(sizeJob);
    root->fromleft = malloc(sizeJob);

    //printf("%s", "passou por aqui");

    //double *d = src;

    if(lo+1 == hi){
        memcpy(root->sum, src + lo * sizeJob, sizeJob);
        root->index = lo;

        //printf ("%lu ", lo);

        //memcpy(&d, &root->sum, sizeJob);

        //printf ("%lf ", *d);
    }

    else
    {

    //worker(root->sum, src + lo *sizeJob, src + (hi-1)*sizeJob);


    size_t mid = (lo+hi)/2;

    //void *aux = malloc(nJob*sizeof(struct node));




    //int tid = omp_get_num_threads();
    //printf("%d", tid);
    //int tid = omp_get_thread_num();
    //printf("%d", tid);

    //#pragma omp single
        #pragma omp task
            upPass(root->right, src, lo, mid, sizeJob, worker);

            //tid = omp_get_thread_num();
            //printf("%d", tid);
    //#pragma omp single
        #pragma omp task
            upPass(root->left, src, mid, hi, sizeJob, worker);
    //#pragma omp barrier
    //}


    #pragma omp taskwait 
     
    worker(root->sum, root->left->sum, root->right->sum);
    
    //memcpy(&d, &root->sum, sizeJob);

    //printf ("%lf ", *d);
  
    }
        
}

void downPass(void *src, void *dest, size_t isleft, struct node* parent, struct node* child, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3))
{

    if(isleft != -1)
        child->fromleft = parent->fromleft;
    else
        worker(child->fromleft, parent->fromleft, parent->left->sum);

    if(child->index != -1)
        worker(dest + child->index * sizeJob, child->fromleft, src + child->index * sizeJob);
    else
    {
    #pragma omp task
    downPass(src, dest, -1, child, child->right, sizeJob, worker);

    #pragma omp task
    downPass(src, dest, 1, child, child->left, sizeJob, worker);

   // #pragma omp taskwait
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
    assert(dest != NULL);
    assert(src != NULL);
    assert(filter != NULL);
    assert(nJob >= 0);
    assert(sizeJob > 0);
    char *d = dest;
    char *s = src;
    int *auxFilter = calloc(nJob, sizeof(int));

#pragma omp parallel for num_threads(8)
    for (int i = 0; i < nJob; i++)
    {
        assert(filter[i] < nJob);
        if (i > auxFilter[filter[i]])
        {
            memcpy(&d[filter[i] * sizeJob], &s[i * sizeJob], sizeJob);
            auxFilter[filter[i]] = i;
        }
    }

    free(auxFilter);
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
