
#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_IO.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_Interop.hpp"
#include <string>
#include <iostream>
#include "Mongoose_Test.hpp"

using namespace Mongoose;

void RunAllTests (
    const std::string inputFile, 
    Options *O
);

int RunTest (
    const std::string inputFile, 
    Options *O,
    int allowedMallocs
);

/* Custom memory management functions allow for memory testing. */
int AllowedMallocs;

/*
const int num_mallocs[][4][3] = { { { 531, 134, 146 },
                                    { 471, 134, 146 },
                                    { 430, 105, 117 },
                                    { 526, 105, 117 } },
                                  { { 531, 134, 146 },
                                    { 471, 200,   1 },
                                    {   1,   1,   1 },
                                    {   1,   1, 117 } } };
                                    */

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

int run_memory_tests()
{
    SuiteSparse_start();

    const std::string inputFile = "../Matrix/bcspwr04.mtx";
    //const std::string inputFile = "../Matrix/Erdos971.mtx";

    Options *O = Options::Create();
    if(!O)
    {
         SuiteSparse_finish();
         return 1; // Return an error if we failed.
    }

    O->doExpensiveChecks = true;

    /* Override SuiteSparse memory management with custom testers. */
    SuiteSparse_config.malloc_func = myMalloc;
    SuiteSparse_config.calloc_func = myCalloc;
    SuiteSparse_config.realloc_func = myRealloc;
    SuiteSparse_config.free_func = myFree;

    RunAllTests(inputFile, O);

    O->~Options();
    SuiteSparse_free(O);

    /* Return success */
    SuiteSparse_finish();

    return 0;
}

void RunAllTests (
    const std::string inputFile,
    Options *O
)
{
    //printf("Running all tests with %d AllowedMallocs\n", allowedMallocs);

   /* 
    * (1 TEST)
    * TRY A BOGUS LOAD
    */
    //RunTest("bogus", O, allowedMallocs);
    
    /* 
     * (12 TESTS)
     * TRY A VARIETY OF MATCHING STRATEGIES AND GUESS CUT STRATEGIES
     */
    MatchingStrategy matchingStrategies[4] = {Random, HEM, HEMPA, HEMDavisPA};
    GuessCutType guessCutStrategies[3] = {Pseudoperipheral_All, Pseudoperipheral_Fast, QP_GradProj};

    for(int c = 0; c < 2; c++)
    {
        O->doCommunityMatching = c;

        for(int i = 0; i < 4; i++)
        {
            O->matchingStrategy = matchingStrategies[i];

            for(int j = 0; j < 3; j++)
            {
                O->guessCutType = guessCutStrategies[j];

                //for(int m = 0; m < num_mallocs[c][i][j]; m++)
                int m = 0;
                while (RunTest(inputFile, O, m) < 1)
                {
                    m += 1;
                }
            }
        }
    }

   /* 
    * (1 TEST)
    * DO A DEEP COPY OF THE GRAPH AND RESET EDGEWEIGHTS
    */
    Graph *DCG = read_graph(inputFile);
    if(DCG)
    {
        cs *csDCG = GraphToCSparse3(DCG, true);
        cs_spfree(csDCG);
        DCG->~Graph();
        SuiteSparse_free(DCG);
    }
}

int RunTest (
    const std::string inputFile, 
    Options *O,
    int allowedMallocs
)
{
    /* Set the # of mallocs that we're allowed. */
    AllowedMallocs = allowedMallocs;

    /* Read and condition the matrix from the MM file. */
    Graph *U = read_graph(inputFile);
    if (!U) return -1;

    ComputeEdgeSeparator(U, O);
    U->~Graph();
    SuiteSparse_free(U);
    return AllowedMallocs;
    //printf("Test Complete! Remaining mallocs: %d\n", AllowedMallocs);
}