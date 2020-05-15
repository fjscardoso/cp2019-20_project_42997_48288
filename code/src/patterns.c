//Trabalho realizado por: 
//Francisco Cardoso, nº 42997
//Nelson Santos, nº 48288
#include <string.h>
#include <assert.h>
#include "patterns.h"
#include <omp.h>

#include <stdio.h>

#define TYPE double

#define THREADS_NUM 8

struct node {
  void* sum;
  void* fromleft;
  size_t index;

  struct node* left;
  struct node* right;
};

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

    assert(dest != NULL);
    assert(src != NULL);
    assert(worker != NULL);
    char *d = dest;
    char *s = src;
    #pragma omp parallel for num_threads(THREADS_NUM)
    for (int i = 0; i < nJob; i++)
    {
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

    int maxWorkers = THREADS_NUM;
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

void upPass(struct node *root, void *src, size_t lo, size_t hi, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3))
{

    root->right = malloc(sizeof(struct node));
    root->left = malloc(sizeof(struct node));
    root->index = -1;
    root->sum = malloc(sizeJob);
    root->fromleft = malloc(sizeJob);

    if (lo + 1 == hi)
    {
        memcpy(root->sum, src + lo * sizeJob, sizeJob);
        root->index = lo;
    }

    else
    {

        size_t mid = (lo + hi) / 2;

        #pragma omp task
        upPass(root->right, src, lo, mid, sizeJob, worker);

        #pragma omp task
        upPass(root->left, src, mid, hi, sizeJob, worker);

        #pragma omp taskwait

        worker(root->sum, root->left->sum, root->right->sum);
    }
}

void downPass(void *src, void *dest, size_t isleft, struct node *parent, struct node *child, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3))
{

    if (isleft != -1)
        child->fromleft = parent->fromleft;
    else
        worker(child->fromleft, parent->fromleft, parent->left->sum);

    if (child->index != -1)
        worker(dest + child->index * sizeJob, child->fromleft, src + child->index * sizeJob);
    else
    {
        #pragma omp task
        downPass(src, dest, -1, child, child->right, sizeJob, worker);

        #pragma omp task
        downPass(src, dest, 1, child, child->left, sizeJob, worker);

    }
}

void scan(void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2, const void *v3))
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(worker != NULL);

    struct node *root = malloc(sizeof(struct node));
    root->right = malloc(sizeof(struct node));
    root->left = malloc(sizeof(struct node));
    root->sum = malloc(sizeJob);
    root->fromleft = malloc(sizeJob);

    root->fromleft = 0;

    #pragma omp parallel num_threads(THREADS_NUM)
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

}

//======================================================================================================================
// PACK
//======================================================================================================================
int packSequential(void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter)
{
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

static void workerAddPack(void *a, const void *b, const void *c)
{
    *(TYPE *)a = *(TYPE *)b + *(TYPE *)c;
}

int pack(void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(filter != NULL);
    assert(nJob >= 0);
    assert(sizeJob > 0);
    char *d = dest;
    char *s = src;
    int pos = 0;

    void *bitsum = malloc((nJob) * sizeof(int));

    scanSequential(bitsum, (void *)filter, nJob, sizeof(int), workerAddPack);

    #pragma omp parallel for num_threads(THREADS_NUM)
    for (int i = 0; i < nJob; i++)
    {

        if (filter[i])
        {
            int x = ((int *)bitsum)[i];
            memcpy(&d[(x - 1) * sizeJob], &s[i * sizeJob], sizeJob);
            pos++;
        }
    }

    free(bitsum);

    return pos;
}

//======================================================================================================================
// GATHER
//======================================================================================================================
void gatherSequential(void *dest, void *src, size_t nJob, size_t sizeJob, const int *filter, int nFilter)
{
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
    assert(dest != NULL);
    assert(src != NULL);
    assert(filter != NULL);
    assert(nJob >= 0);
    assert(sizeJob > 0);
    assert(nFilter >= 0);
    char *d = dest;
    char *s = src;
    #pragma omp parallel for num_threads(THREADS_NUM)
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

    #pragma omp parallel for num_threads(THREADS_NUM)
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
void pipelineSequential(void *dest, void *src, size_t nJob, size_t sizeJob, void (*workerList[])(void *v1, const void *v2), size_t nWorkers)
{
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

// Implementation used from: Solihin, Yan. (2015). "Fundamentals of Parallel Computer Architecture". Pág 40
//Calculate number of anti-diagonals
//foreach anti-diagonal do {
//calculate number of points of anti-diagonal
//foreach point in current ant-diagonal do {
//compute current point in matrix
//}
void pipeline(void *dest, void *src, size_t nJob, size_t sizeJob, void (*workerList[])(void *v1, const void *v2), size_t nWorkers)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(workerList != NULL);
    assert(nJob >= 0);
    assert(sizeJob > 0);
    char *d = dest;
    char *s = src;

    #pragma omp parallel for num_threads(THREADS_NUM)
    for (int i = 0; i < nJob; i++)
    {
        memcpy(&d[i * sizeJob], &s[i * sizeJob], sizeJob);
    }
    int k = 0;
    int row = k;
    int j = 0;
    int diagonals = (nJob + nWorkers) - 1;
   
    for (k = 0; k < diagonals; k++)
    {
        row = k;
    //#pragma omp for schedule(dynamic)
    #pragma omp parallel for ordered
        for (j = 0; j < nWorkers; j++)
        {
            if (row >= 0 && row < nJob)
            {
                #pragma omp ordered
                workerList[j](&d[row * sizeJob], &d[row * sizeJob]);
            }
        }
    }
}

//======================================================================================================================
// FARM
//======================================================================================================================
void farmSequential(void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2), size_t nWorkers)
{
    map(dest, src, nJob, sizeJob, worker); 
}

void newTask(void *dest, void *src, size_t sizeJob, size_t startJob, size_t endJob, void (*worker)(void *v1, const void *v2))
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(worker != NULL);
    char *d = dest;
    char *s = src;

    for (int i = startJob; i < endJob; i++)
    {
        worker(&d[i * sizeJob], &s[i * sizeJob]);
    }
}

void farm(void *dest, void *src, size_t nJob, size_t sizeJob, void (*worker)(void *v1, const void *v2), size_t nWorkers)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(worker != NULL);
    char *d = dest;
    char *s = src;
    int maxWorkers = nWorkers;
    int jobsPerWorker = nJob / maxWorkers;
    int jobsRemainder = nJob % maxWorkers;

    #pragma omp parallel
    {
    #pragma omp master
        {
            for (int k = 0; k < maxWorkers; k++)
            {
                int startJob = k * jobsPerWorker;
                int endJob = startJob + jobsPerWorker;
                endJob = k == maxWorkers - 1 ? (endJob + jobsRemainder) : endJob;
                #pragma omp task
                newTask(d, s, sizeJob, startJob, endJob, worker);
            }
        }
    }
}
