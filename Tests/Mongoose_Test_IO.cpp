#include <string>
#include "Mongoose_IO.hpp"
#include "Mongoose_Conditioning.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include <cassert>
#include "Mongoose_Test.hpp"
#include <iostream>
#include <fstream>

using namespace Mongoose;

typedef struct mm_file_data
{
    const std::string filename;
    const Int n;
    const Int nz;
} mm_file;

mm_file mm_data[] = {
    {"Pd",       1337,  2982},
    {"bcspwr01",   39,    92},
    {"bcspwr02",   49,   118},
    {"bcspwr03",  118,   358},
    {"bcspwr04",  274,  1338},
    {"bcspwr05",  443,  1180},
    {"bcspwr06", 1454,  3846},
    {"bcspwr07", 1612,  4212},
    {"bcspwr08", 1624,  4426},
    {"bcspwr09", 1723,  4788},
    {"bcspwr10", 5300, 16542}
};

void runIOTests()
{
    Logger::log(Test, "Running I/O Test...");

    for (int k = 0; k < 11; k++)
    {
        // Given a symmetric matrix
        Options *options;
        Graph *G;
        
        options = Options::Create();
        if (!options)
        {
            // Ran out of memory
            SuiteSparse_free(options);
            continue;
        }
        
        G = readGraph("../Matrix/" + mm_data[k].filename + ".mtx");

        if (!G)
        {
            // Ran out of memory
            SuiteSparse_free(options);
            SuiteSparse_free(G);
            continue;
        }

        // The number of vertices should be correct
        assert (G->n == mm_data[k].n);

        // The number of edges should be correct
        assert (G->nz == mm_data[k].nz);

        // An edge separator should be computed with default options
        int error = ComputeEdgeSeparator(G, options);

        if (error)
        {
            // Error occurred
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
        }

        // The graph sizes should not change
        assert (G->n == mm_data[k].n);
        assert (G->nz == mm_data[k].nz);

        G->~Graph();
        SuiteSparse_free(G);
        SuiteSparse_free(options);
    }

    // Corner cases
    Graph *G;

    // Bad header
    G = readGraph("../Matrix/bad_header.mtx");
    assert (G == NULL);
    SuiteSparse_free(G);

    // Bad matrix type
    G = readGraph("../Matrix/bad_matrix_type.mtx");
    assert (G == NULL);
    SuiteSparse_free(G);

    // Bad dimensions
    G = readGraph("../Matrix/bad_dimensions.mtx");
    assert (G == NULL);
    SuiteSparse_free(G);

    // Rectangular matrix
    G = readGraph("../Matrix/Trec4.mtx");
    assert (G == NULL);
    SuiteSparse_free(G);

    // Other tests
    // TODO: Move these to another file

    Logger::log(Test, "I/O Test Completed Successfully");
}