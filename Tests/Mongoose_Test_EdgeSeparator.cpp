#include <string>
#include "Mongoose_IO.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_Test.hpp"
#include <fstream>

using namespace Mongoose;

int runEdgeSeparatorTest(const std::string &inputFile, const double targetSplit)
{
    LogTest("Running Edge Separator Test on " << inputFile);
        
    // Given a symmetric matrix...
    Options *options;
    Graph *graph;
    
    options = Options::Create();
    if (!options)
    {
        // Ran out of memory
        LogTest("Error creating Options struct in Edge Separator Test");
        return EXIT_FAILURE;
    }

    options->targetSplit = targetSplit;
    
    // Read graph from file
    graph = readGraph(inputFile);

    if (!graph)
    {
        // Ran out of memory
        LogTest("Error reading Graph from file in Edge Separator Test");
        options->~Options();
        return EXIT_FAILURE;
    }

    // An edge separator should be computed with default options
    int error = ComputeEdgeSeparator(graph, options);

    options->~Options();

    if (error)
    {
        // Error occurred
        LogTest("Error computing edge separator in Edge Separator Test");
        return EXIT_FAILURE;
    }
    else
    {
        // The graph should be partitioned
        assert (graph->partition != NULL);
        int count = 0;
        for (int i = 0; i < graph->n; i++)
        {
            bool equals_0 = (graph->partition[i] == 0);
            bool equals_1 = (graph->partition[i] == 1);
            assert(equals_0 != equals_1);

            count += graph->partition[i];
        }

        double split = (double) count / (double) graph->n;
        double target = targetSplit;
        if (split > 0.5)
        {
            split = 1 - split;
        }
        if (targetSplit > 0.5)
        {
            target = 1 - target;
        }

        Logger::printTimingInfo();
        LogTest("Cut Properties:");
        LogTest("  Cut Cost:  " << graph->cutCost);
        LogTest("  Imbalance: " << graph->imbalance);
    }

    graph->~Graph();

    LogTest("Edge Separator Test Completed Successfully");

    return EXIT_SUCCESS;
}
