
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

int run_memory_test(const std::string inputFile)
{
    SuiteSparse_start();

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

int run_memory_tests()
{
    const std::string inputFile = "../Matrix/bcspwr04.mtx";

    run_memory_test(inputFile);
}

void RunAllTests (
    const std::string inputFile,
    Options *O
)
{
    Logger::setDebugLevel(Info);
    Logger::log(Info, "Running Memory Test...");

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

                int m = 0;
                int remainingMallocs;
                do {
                    remainingMallocs = RunTest(inputFile, O, m);
                    //Logger::log(Info, "Remaining Mallocs: " + std::to_string(remainingMallocs));
                    if (remainingMallocs == -1)
                    {
                        // Error!
                        Logger::log(Info, "Terminating Memory Test Early");
                        return;
                    }
                    m += 1;
                } while (remainingMallocs < 1);
            }
        }
    }

   /* 
    * DO A DEEP COPY OF THE GRAPH AND RESET EDGEWEIGHTS
    */
    Graph *DCG = readGraph(inputFile);
    if(DCG)
    {
        cs *csDCG = GraphToCSparse3(DCG, true);
        cs_spfree(csDCG);
        DCG->~Graph();
        SuiteSparse_free(DCG);
    }

    Logger::log(Info, "Memory Test Completed Successfully");
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
    Graph *U = readGraph(inputFile);
    if (!U) return AllowedMallocs;

    ComputeEdgeSeparator(U, O);
    U->~Graph();
    SuiteSparse_free(U);

    return AllowedMallocs;
}