
#define LOG_ERROR 1
#define LOG_WARN 1
#define LOG_INFO 0
#define LOG_TEST 1

#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_IO.hpp"
#include "Mongoose_Interop.hpp"
#include "Mongoose_EdgeSeparator.hpp"

using namespace Mongoose;

int main(int argn, char** argv)
{
    (void)argn; // Unused variable
    (void)argv; // Unused variable

    SuiteSparse_start();

    // Set Logger to report all messages and turn off timing info
    Logger::setDebugLevel(All);
    Logger::setTimingFlag(false);

    // Test with NULL graph
    ComputeEdgeSeparator(NULL);

    Graph *G = readGraph("../Matrix/bcspwr02.mtx");

    // Test with no options struct
    ComputeEdgeSeparator(G);

    // Test with NULL options struct
    Options *O = NULL;
    ComputeEdgeSeparator(G, O);


    O = Options::Create();

    // Test with invalid coarsenLimit
    O->coarsenLimit = 0;
    ComputeEdgeSeparator(G, O);
    O->coarsenLimit = 50;

    // Test with invalid davisBrotherlyThreshold
    O->davisBrotherlyThreshold = -1;
    ComputeEdgeSeparator(G, O);
    O->davisBrotherlyThreshold = 2.0;

    // Test with invalid numDances
    O->numDances = -1;
    ComputeEdgeSeparator(G, O);
    O->numDances = 1;

    // Test with invalid fmSearchDepth
    O->fmSearchDepth = -1;
    ComputeEdgeSeparator(G, O);
    O->fmSearchDepth = 50;

    // Test with invalid fmConsiderCount
    O->fmConsiderCount = -1;
    ComputeEdgeSeparator(G, O);
    O->fmConsiderCount = 3;

    // Test with invalid fmMaxNumRefinements
    O->fmMaxNumRefinements = -1;
    ComputeEdgeSeparator(G, O);
    O->fmMaxNumRefinements = 20;

    // Test with invalid gradProjTolerance
    O->gradProjTolerance = -1;
    ComputeEdgeSeparator(G, O);
    O->gradProjTolerance = 0.001;

    // Test with invalid gradProjIterationLimit
    O->gradprojIterationLimit = -1;
    ComputeEdgeSeparator(G, O);
    O->gradprojIterationLimit = 50;

    // Test with invalid targetSplit
    O->targetSplit = 1.2;
    ComputeEdgeSeparator(G, O);
    O->targetSplit = 0.4;

    // Test with invalid tolerance
    O->softSplitTolerance = -1;
    ComputeEdgeSeparator(G, O);
    O->softSplitTolerance = 0.01;

    // Test with no QP
    O->useQPGradProj = false;
    ComputeEdgeSeparator(G, O);
    assert(G->partition != NULL);
    O->useQPGradProj = true;

    // Test with no FM
    O->useFM = false;
    ComputeEdgeSeparator(G, O);
    assert(G->partition != NULL);
    O->useFM = true;

    // Test with no coarsening
    O->coarsenLimit = 1E15;
    ComputeEdgeSeparator(G,O);
    assert(G->partition != NULL);

    // Test with x = NULL (assume pattern matrix)
    G->x = NULL;
    ComputeEdgeSeparator(G,O);
    assert(G->partition != NULL);
    O->coarsenLimit = 50;

    O->~Options();
    G->~Graph();

    SuiteSparse_finish();

    return 0;
}
