
#include "Mongoose_Internal.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_IO.hpp"
#include "Mongoose_Logger.hpp"

#include <fstream>

using namespace Mongoose;

int main(int argn, const char **argv)
{
    SuiteSparse_start();

    clock_t t;
    
    // Set Logger to report only Error messages
    Logger::setDebugLevel(Error);

    if (argn < 2 || argn > 3)
    {
        // Wrong number of arguments - return error
        LogError("Usage: Demo <MM-input-file.mtx> [output-file]");
        SuiteSparse_finish();
        return EXIT_FAILURE;
    }

    // Read in input file name
    std::string inputFile = std::string(argv[1]);

    std::string outputFile;
    if (argn == 3)
    {
        outputFile = std::string(argv[2]);
    }
    else
    {
        outputFile = "mongoose_out.txt";
    }

    // Turn timing information on
    Logger::setTimingFlag(true);

    Options *options = Options::Create();
    if (!options)
    {
        // Ran out of memory
        LogError("Error creating Options struct");
        return EXIT_FAILURE;
    }

    Graph *G = readGraph(inputFile);

    if (!G)
    {
        // Ran out of memory or problem reading the graph from file
        LogError("Error reading Graph from file");
        return EXIT_FAILURE;
    }

    // An edge separator should be computed with default options
    t = clock();
    int error = ComputeEdgeSeparator(G, options);
    t = clock() - t;

    if (error)
    {
        // Error occurred
        LogError("Error computing edge separator");
        G->~Graph();
        return EXIT_FAILURE;
    }
    else
    {
        double test_time = ((double) t)/CLOCKS_PER_SEC;
        std::cout << "Total Edge Separator Time: " << test_time << "s\n";
        Logger::printTimingInfo();
        std::cout << "Cut Properties:\n";
        std::cout << " Cut Size:  " << G->cutSize << "\n";
        std::cout << " Cut Cost:  " << G->cutCost << "\n";
        std::cout << " Imbalance: " << G->imbalance << "\n";

        // Write results to file
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
            ofs << "  \"CutSize\": " << G->cutSize << "," << std::endl;
            ofs << "  \"CutCost\": " << G->cutCost << "," << std::endl;
            ofs << "  \"Imbalance\": " << G->imbalance << std::endl;
            ofs << "}" << std::endl;

            ofs << std::endl;
            for (Int i = 0; i < G->n; i++)
            {
                ofs << i << " " << G->partition[i] << std::endl;
            }
            ofs << std::endl;

            ofs.close();
        }
    }

    G->~Graph();

    SuiteSparse_finish();

    return 0 ;
}
