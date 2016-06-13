#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>

#include "mongoose_io.hpp"
#include "mongoose_conditioning.hpp"

using namespace Mongoose;

typedef struct mm_file_data
{
    const std::string filename;
    const Int n;
    const Int nz;
} mm_file;

SCENARIO( "Matrix Market files can be used as input", "[I/O]" )
{
    mm_file mm_data[] = {
        {"bcspwr01.mtx",   39,    85},
        {"bcspwr02.mtx",   49,   108},
        {"bcspwr03.mtx",  118,   297},
        {"bcspwr04.mtx",  274,   943},
        {"bcspwr05.mtx",  443,  1033},
        {"bcspwr06.mtx", 1454,  3377},
        {"bcspwr07.mtx", 1612,  3718},
        {"bcspwr08.mtx", 1624,  3837},
        {"bcspwr09.mtx", 1723,  4117},
        {"bcspwr10.mtx", 5300, 13571}
    };

    GIVEN( "A symmetric matrix" )
    {
        Options *options;
        Graph *G;

        for(int k = 0; k < 10; k++)
        {
            options = Options::Create();
            G = read_graph(("../Matrix/" + mm_data[k].filename).c_str());

            REQUIRE( G->n  == mm_data[k].n );
            REQUIRE( G->nz == mm_data[k].nz );

            WHEN( "an edge separator is computed with default options" )
            {
                ComputeEdgeSeparator(G, options);
                THEN( "the graph sizes should not change" )
                {
                    REQUIRE( G->n  == mm_data[k].n );
                    REQUIRE( G->nz == mm_data[k].nz );
                }
                AND_THEN( "the graph should be partitioned" )
                {
                    
                }
        }
        
    }
}