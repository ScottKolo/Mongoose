
#define LOG_ERROR 1
#define LOG_WARN 1
#define LOG_INFO 0
#define LOG_TEST 1

#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_IO.hpp"

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
    (void)argn; // Unused variable
    (void)argv; // Unused variable

    SuiteSparse_start();

    // Set Logger to report all messages and turn off timing info
    Logger::setDebugLevel(All);
    Logger::setTimingFlag(false);

    // Test Graph(n, nz) static constructor
    Graph *G2 = Graph::Create(10, 20, true);

    G2->clearMarkArray(LONG_MAX);
    Int markValue = G2->getMarkValue();
    assert(markValue == 1);

    G2->clearMarkArray(LONG_MAX-1);
    G2->clearMarkArray();
    markValue = G2->getMarkValue();
    assert(markValue == 1);
    G2->~Graph();

    MM_typecode matcode;
    cs *M4 = readMatrix("../Matrix/bcspwr01.mtx", matcode);
    M4->x = NULL;
    Graph *G7 = Graph::Create(M4);
    assert(G7 != NULL);
    G7->~Graph();

    // Tests to increase coverage
    /* Override SuiteSparse memory management with custom testers. */
    SuiteSparse_config.malloc_func = myMalloc;
    SuiteSparse_config.calloc_func = myCalloc;
    SuiteSparse_config.realloc_func = myRealloc;
    SuiteSparse_config.free_func = myFree;

    // Simulate failure to allocate return arrays
    AllowedMallocs = 0;
    Graph *G3 = Graph::Create(10, 20, true);
    assert(G3 == NULL);

    AllowedMallocs = 4;
    Graph *G4 = Graph::Create(10, 20, true);
    assert(G4 == NULL);

    AllowedMallocs = 8;
    Graph *G5 = Graph::Create(10, 20, true);
    assert(G5 == NULL);

    AllowedMallocs = 14;
    Graph *G6 = Graph::Create(10, 20, true);
    assert(G6 == NULL);

    SuiteSparse_finish();

    return 0;
}
