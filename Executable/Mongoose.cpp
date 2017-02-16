
#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_IO.hpp"

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
        LogError("Usage: Demo <MM-input-file.mtx>");
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
        LogError("Error creating Options struct");
        SuiteSparse_free(options);
        return 1;
    }
    //options->matchingStrategy = LabelPropagation;

    Graph *G = readGraph(inputFile);

    if (!G)
    {
        // Ran out of memory or problem reading the graph from file
        LogError("Error reading Graph from file");
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
        LogError("Error computing edge separator in Performance Test");
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
        std::cout << " Cut Size:  " << cutSize << "\n";
        std::cout << " Cut Cost:  " << G->cutCost << "\n";
        std::cout << " Imbalance: " << G->imbalance << "\n";
    }

    G->~Graph();
    SuiteSparse_free(G);
    SuiteSparse_free(options);

    SuiteSparse_finish();

    return 0 ;
}
