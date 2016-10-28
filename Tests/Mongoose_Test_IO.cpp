#include <string>
#include "Mongoose_IO.hpp"
#include <cassert>
#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"

using namespace Mongoose;

void runIOTest(const std::string inputFile, bool validGraph)
{
    Logger::log(Test, "Running I/O Test on " + inputFile);

    Graph *G = readGraph(inputFile);

    if (validGraph)
    {
        assert(G != NULL);    // A valid graph should not be null
        assert(G->n > 0);     // A valid graph should have 
        assert(G->nz >= 0);   // At least 1 edge
        assert(G->p != NULL); // Column pointers should not be null
        assert(G->i != NULL); // Row numbers should not be null
        G->~Graph();
        SuiteSparse_free(G);
    }
    else
    {
        assert(G == NULL);
    }
    
    Logger::log(Test, "I/O Test Completed Successfully");
}