#include <string>
#include "Mongoose_IO.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_Test.hpp"
#include <fstream>

using namespace Mongoose;

void runEdgeSeparatorTest(const std::string &inputFile, const double targetSplit)
{
    LogTest("Running Edge Separator Test on " << inputFile);
        
    // Given a symmetric matrix...
    Options *options;
    Graph *G;
    
    options = Options::Create();
    if (!options)
    {
        // Ran out of memory
        LogTest("Error creating Options struct in Edge Separator Test");
        SuiteSparse_free(options);
        assert(false);
    }

    options->targetSplit = targetSplit;
    
    // Read graph from file
    G = readGraph(inputFile);

    if (!G)
    {
        // Ran out of memory
        LogTest("Error reading Graph from file in Edge Separator Test");
        SuiteSparse_free(options);
        SuiteSparse_free(G);
        assert(false);
    }

    // An edge separator should be computed with default options
    int error = ComputeEdgeSeparator(G, options);

    if (error)
    {
        // Error occurred
        LogTest("Error computing edge separator in Edge Separator Test");
        assert(false);
    }
    else
    {
        // The graph should be partitioned
        assert (G->partition != NULL);
        int count = 0;
        for (int i = 0; i < G->n; i++)
        {
            bool equals_0 = (G->partition[i] == 0);
            bool equals_1 = (G->partition[i] == 1);
            assert(equals_0 != equals_1);

            count += G->partition[i];
        }
        double tolerance = options->tolerance;
        double split = (double) count / (double) G->n;
        double target = targetSplit;
        if (split > 0.5)
        {
            split = 1 - split;
        }
        if (targetSplit > 0.5)
        {
            target = 1 - target;
        }
        LogTest("Split: " << split << ", Target: " << target << ", Tolerance: " << tolerance);
        std::cout << "Split: " << split << ", Target: " << target << ", Tolerance: " << tolerance << std::endl;
        assert (split <= target + 100*tolerance);
        assert (split >= target - 100*tolerance);

        Logger::printTimingInfo();
        LogTest("Cut Properties:");
        LogTest("  Cut Cost:  " << G->cutCost);
        LogTest("  Imbalance: " << G->imbalance);
    }

    G->~Graph();
    SuiteSparse_free(G);
    SuiteSparse_free(options);

    LogTest("Edge Separator Test Completed Successfully");
}
