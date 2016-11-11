#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_IO.hpp"
#include "Mongoose_Interop.hpp"
#include "Mongoose_CSparse.hpp"
#include <string>
#include <stdlib.h>

using namespace Mongoose;

/* Custom memory management functions allow for memory testing. */
int AllowedMallocs;

void *myMalloc(size_t size)
{
    if(AllowedMallocs <= 0) return NULL;
    AllowedMallocs--;
    return malloc(size);
}

void *myCalloc(size_t count, size_t size)
{
    if(AllowedMallocs <= 0) return NULL;
    AllowedMallocs--;
    return calloc(count, size);
}

void *myRealloc(void *ptr, size_t newSize)
{
    if(AllowedMallocs <= 0) return NULL;
    AllowedMallocs--;
    return realloc(ptr, newSize);
}

void myFree(void *ptr)
{
    if(ptr != NULL) free(ptr);
}

int main(int argn, char** argv)
{
    SuiteSparse_start();

    // Set Logger to report all messages and turn off timing info
    Logger::setDebugLevel(All);
    Logger::setTimingFlag(false);

    // Test default constructor
    Graph G1;

    // Test Graph(n, nz) static constructor
    Graph *G2 = Graph::Create(10, 20);

    cs *M1 = GraphToCSparse3(G2, false);
    assert(M1 != NULL);
    cs *M2 = GraphToCSparse3(G2, true);
    assert(M2 != NULL);

    SuiteSparse_free(M1);
    M2->x = NULL;
    M2->p = NULL;
    M2->i = NULL;
    SuiteSparse_free(M2);

    MM_typecode matcode;
    cs *M4 = readMatrix("../Matrix/bcspwr01.mtx", matcode);
    M4->x = NULL;
    Graph *G7 = CSparse3ToGraph(M4, 0, 0);
    assert(G7 != NULL);
    SuiteSparse_free(G7);

    // Tests to increase coverage
    /* Override SuiteSparse memory management with custom testers. */
    SuiteSparse_config.malloc_func = myMalloc;
    SuiteSparse_config.calloc_func = myCalloc;
    SuiteSparse_config.realloc_func = myRealloc;
    SuiteSparse_config.free_func = myFree;

    // Simulate failure to allocate return arrays
    AllowedMallocs = 2;
    cs *M3 = GraphToCSparse3(G2, true);
    assert(M3 == NULL);
    SuiteSparse_free(M3);
    SuiteSparse_free(G2);

    AllowedMallocs = 0;
    Graph *G3 = Graph::Create(10, 20);
    assert(G3 == NULL);
    SuiteSparse_free(G3);

    AllowedMallocs = 4;
    Graph *G4 = Graph::Create(10, 20);
    assert(G4 == NULL);
    SuiteSparse_free(G4);

    AllowedMallocs = 8;
    Graph *G5 = Graph::Create(10, 20);
    assert(G5 == NULL);
    SuiteSparse_free(G5);

    AllowedMallocs = 14;
    Graph *G6 = Graph::Create(10, 20);
    assert(G6 == NULL);
    SuiteSparse_free(G6);

    SuiteSparse_finish();

    return 0;
}