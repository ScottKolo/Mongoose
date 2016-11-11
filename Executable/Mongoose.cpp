
#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_CSparse.hpp"
#include "Mongoose_IO.hpp"
#include <ctime>

using namespace Mongoose;

int main(int argn, const char **argv)
{
    SuiteSparse_start();

    clock_t t;
    
    // Set Logger to report only Error messages
    Logger::setDebugLevel(Error);

    if (argn != 2)
    {
        // Wrong number of arguments - return error
        Logger::log(Error, "Usage: Demo <MM-input-file.mtx>");
        SuiteSparse_finish();
        return 1;
    }

    // Read in input file name
    std::string inputFile = std::string(argv[1]);
    
    // Turn timing information on
    Logger::setTimingFlag(true);

    Options *options = Options::Create();
    if (!options)
    {
        // Ran out of memory
        Logger::log(Error, "Error creating Options struct");
        SuiteSparse_free(options);
        return 1;
    }

    Graph *G = readGraph(inputFile);

    if (!G)
    {
        // Ran out of memory or problem reading the graph from file
        Logger::log(Error, "Error reading Graph from file");
        SuiteSparse_free(options);
        SuiteSparse_free(G);
        return 1;
    }

    // An edge separator should be computed with default options
    t = clock();
    int error = ComputeEdgeSeparator(G, options);
    t = clock() - t;

    if (error)
    {
        // Error occurred
        Logger::log(Error, "Error computing edge separator in Performance Test");
        SuiteSparse_free(options);
        SuiteSparse_free(G);
        return 1;
    }
    else
    {
        long cutSize = 0;
        for(int i = 0; i < G->n; i++)
        {
            cutSize += G->externalDegree[i];
        }
        cutSize = cutSize / 2;
        double test_time = ((double) t)/CLOCKS_PER_SEC;
        Logger::log(All, "Total Edge Separator Time: " + std::to_string(test_time) + "s");
        Logger::printTimingInfo();
        Logger::log(All, "Cut Properties:");
        Logger::log(All, " Cut Size:  " + std::to_string(cutSize));
        Logger::log(All, " Cut Cost:  " + std::to_string(G->cutCost));
        Logger::log(All, " Imbalance: " + std::to_string(G->imbalance));
    }

    G->~Graph();
    SuiteSparse_free(G);
    SuiteSparse_free(options);

    SuiteSparse_finish();

    return 0 ;
}
