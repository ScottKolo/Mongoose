#include <string>
#include "Mongoose_IO.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include <cassert>
#include "Mongoose_Test.hpp"
#include <iostream>
#include <fstream>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */

using namespace Mongoose;

void runPerformanceTest(const std::string inputFile, const std::string outputFile)
{
    Options *options;
    Graph *G;
    clock_t t;
    
    options = Options::Create();

    if (!options)
    {
        // Ran out of memory
        Logger::test() << "Error creating Options struct in Performance Test\n";
        SuiteSparse_free(options);
        assert(false);
    }

    G = readGraph(inputFile);

    if (!G)
    {
        // Ran out of memory
        Logger::test() << "Error reading Graph from file in Performance Test\n";
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
        Logger::test() << "Error computing edge separator in Performance Test\n";
        SuiteSparse_free(options);
        SuiteSparse_free(G);
        assert(false);
    }
    else
    {
        double test_time = ((double) t)/CLOCKS_PER_SEC;
        Logger::test() << "Total Edge Separator Time: " << test_time << "s\n";
        Logger::printTimingInfo();
        Logger::test() << "Cut Properties:\n";
        Logger::test() << " Cut Cost:  " << G->cutCost << "\n";
        Logger::test() << " Imbalance: " << G->imbalance << "\n";
        
        if (!outputFile.empty())
        {
            Logger::test() << "Writing results to file: " << outputFile << "\n";
            std::ofstream ofs (outputFile.c_str(), std::ofstream::out);
            ofs << "{" << std::endl;
            ofs << "  \"InputFile\": \"" << inputFile << "\"," << std::endl;
            ofs << "  \"Timing\": {" << std::endl;
            ofs << "    \"Total\": " << test_time << "," << std::endl;
            ofs << "    \"Matching\": " << Logger::getTime(MatchingTiming) << "," << std::endl;
            ofs << "    \"Coarsening\": " << Logger::getTime(CoarseningTiming) << "," << std::endl;
            ofs << "    \"Refinement\": " << Logger::getTime(RefinementTiming) << "," << std::endl;
            ofs << "    \"FM\": " << Logger::getTime(FMTiming) << "," << std::endl;
            ofs << "    \"QP\": " << Logger::getTime(QPTiming) << "," << std::endl;
            ofs << "    \"IO\": " << Logger::getTime(IOTiming) << std::endl;
            ofs << "  }," << std::endl;
            ofs << "  \"CutSize\": " << G->cutCost << "," << std::endl;
            ofs << "  \"Imbalance\": " << G->imbalance << std::endl;
            ofs << "}" << std::endl;
            ofs.close();
        }
    }

    G->~Graph();
    SuiteSparse_free(G);
    SuiteSparse_free(options);
}