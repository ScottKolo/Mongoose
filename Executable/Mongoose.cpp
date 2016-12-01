
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
        Logger::error() << "Usage: Demo <MM-input-file.mtx>\n";
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
        Logger::error() << "Error creating Options struct\n";
        SuiteSparse_free(options);
        return 1;
    }

    Graph *G = readGraph(inputFile);

    if (!G)
    {
        // Ran out of memory or problem reading the graph from file
        Logger::error() << "Error reading Graph from file\n";
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
        Logger::error() << "Error computing edge separator in Performance Test\n";
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
        std::cout << "Total Edge Separator Time: " << test_time << "s\n";
        Logger::printTimingInfo();
        std::cout << "Cut Properties:\n";
        std::cout << " Cut Size:  " << cutSize;
        std::cout << " Cut Cost:  " << G->cutCost;
        std::cout << " Imbalance: " << G->imbalance;
    }

    G->~Graph();
    SuiteSparse_free(G);
    SuiteSparse_free(options);

    SuiteSparse_finish();

    return 0 ;
}
