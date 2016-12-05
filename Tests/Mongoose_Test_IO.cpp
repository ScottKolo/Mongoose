#include <string>
#include "Mongoose_IO.hpp"
#include <cassert>
#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"

using namespace Mongoose;

void runIOTest(const std::string &inputFile, bool validGraph)
{
    Logger::test() << "Running I/O Test on " << inputFile << "\n";

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

    // Also try with C-style string
    Graph *G2 = readGraph(inputFile.c_str());

    if (validGraph)
    {
        assert(G2 != NULL);    // A valid graph should not be null
        assert(G2->n > 0);     // A valid graph should have 
        assert(G2->nz >= 0);   // At least 1 edge
        assert(G2->p != NULL); // Column pointers should not be null
        assert(G2->i != NULL); // Row numbers should not be null
        G2->~Graph();
        SuiteSparse_free(G2);
    }
    else
    {
        assert(G2 == NULL);
    }
    
    Logger::test() << "I/O Test Completed Successfully\n";
}