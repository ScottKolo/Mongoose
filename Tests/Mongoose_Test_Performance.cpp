#include <string>
#include "Mongoose_IO.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_Test.hpp"
#include <fstream>

using namespace Mongoose;

int runPerformanceTest(const std::string &inputFile, const std::string &outputFile)
{
    Options *options;
    Graph *G;
    clock_t t;
    
    options = Options::Create();

    if (!options)
    {
        // Ran out of memory
        LogTest("Error creating Options struct in Performance Test");
        SuiteSparse_free(options);
        return EXIT_FAILURE;
    }

    G = readGraph(inputFile);

    if (!G)
    {
        // Ran out of memory
        LogTest("Error reading Graph from file in Performance Test");
        SuiteSparse_free(options);
        return EXIT_FAILURE;
    }

    // An edge separator should be computed with default options
    t = clock();
    int error = ComputeEdgeSeparator(G, options);
    t = clock() - t;

    if (error)
    {
        // Error occurred
        LogTest("Error computing edge separator in Performance Test");
        SuiteSparse_free(options);
        G->~Graph();
        return EXIT_FAILURE;
    }
    else
    {
        double test_time = ((double) t)/CLOCKS_PER_SEC;
        LogTest("Total Edge Separator Time: " << test_time << "s");
        Logger::printTimingInfo();
        LogTest("Cut Properties:");
        LogTest(" Cut Cost:  " << G->cutCost);
        LogTest(" Imbalance: " << G->imbalance);
        
        if (!outputFile.empty())
        {
            LogTest("Writing results to file: " << outputFile);
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
    SuiteSparse_free(options);

    return EXIT_SUCCESS;
}