#include <string>
#include "Mongoose_IO.hpp"
#include "Mongoose_Conditioning.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include <cassert>
#include "Mongoose_Test.hpp"
#include <iostream>
#include <fstream>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */

using namespace Mongoose;

void runPerformanceTest(const std::string inputFile)
{
    // Given a symmetric matrix
    Options *options;
    Graph *G;
    clock_t t;
    
    options = Options::Create();

    if (!options)
    {
        // Ran out of memory
        Logger::log(Test, "Error creating Options struct in Performance Test");
        SuiteSparse_free(options);
        assert(false);
    }

    G = readGraph(inputFile);

    if (!G)
    {
        // Ran out of memory
        Logger::log(Test, "Error reading Graph from file in Performance Test");
        SuiteSparse_free(options);
        SuiteSparse_free(G);
        assert(false);
    }

    // An edge separator should be computed with default options
    t = clock();
    int error = ComputeEdgeSeparator(G, options);
    t = clock() - t;

    if (error)
    {
        // Error occurred
        Logger::log(Test, "Error computing edge separator in Performance Test");
        SuiteSparse_free(options);
        SuiteSparse_free(G);
        assert(false);
    }
    else
    {
        double test_time = ((double) t)/CLOCKS_PER_SEC;
        Logger::log(Test, "Total Edge Separator Time: " + std::to_string(test_time) + "s");
        Logger::printTimingInfo();
    }

    G->~Graph();
    SuiteSparse_free(G);
    SuiteSparse_free(options);
}