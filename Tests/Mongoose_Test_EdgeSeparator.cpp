#include <string>
#include "Mongoose_IO.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include <cassert>
#include "Mongoose_Test.hpp"
#include <iostream>
#include <fstream>

using namespace Mongoose;

void runEdgeSeparatorTest(const std::string &inputFile)
{
    Logger::test() << "Running Edge Separator Test on " << inputFile << "\n";
        
    // Given a symmetric matrix...
    Options *options;
    Graph *G;
    
    options = Options::Create();
    if (!options)
    {
        // Ran out of memory
        Logger::test() << "Error creating Options struct in Edge Separator Test\n";
        SuiteSparse_free(options);
        assert(false);
    }
    
    // Read graph from file
    G = readGraph(inputFile);

    if (!G)
    {
        // Ran out of memory
        Logger::test() << "Error reading Graph from file in Edge Separator Test\n";
        SuiteSparse_free(options);
        SuiteSparse_free(G);
        assert(false);
    }

    // An edge separator should be computed with default options
    int error = ComputeEdgeSeparator(G, options);

    if (error)
    {
        // Error occurred
        Logger::test() << "Error computing edge separator in Edge Separator Test\n";
        assert(false);
    }
    else
    {
        // The graph should be partitioned
        assert (G->partition != NULL);
        for (int i = 0; i < G->n; i++)
        {
            bool equals_0 = (G->partition[i] == 0);
            bool equals_1 = (G->partition[i] == 1);
            assert(equals_0 != equals_1);
        }

        Logger::printTimingInfo();
        Logger::test() << "Cut Properties:\n";
        Logger::test() << "  Cut Cost:  " << G->cutCost << "\n";
        Logger::test() << "  Imbalance: " << G->imbalance << "\n";
    }

    G->~Graph();
    SuiteSparse_free(G);
    SuiteSparse_free(options);

    Logger::test() << "Edge Separator Test Completed Successfully\n";
}