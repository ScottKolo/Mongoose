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
        {"bcspwr01.mtx",   39,    92},
        {"bcspwr02.mtx",   49,   118},
        {"bcspwr03.mtx",  118,   358},
        {"bcspwr04.mtx",  274,  1338},
        {"bcspwr05.mtx",  443,  1180},
        {"bcspwr06.mtx", 1454,  3846},
        {"bcspwr07.mtx", 1612,  4212},
        {"bcspwr08.mtx", 1624,  4426},
        {"bcspwr09.mtx", 1723,  4788},
        {"bcspwr10.mtx", 5300, 16542}
    };

    GIVEN( "A symmetric matrix bcspwr10.mtx" )
    {
        Options *options;
        Graph *G;
        
        options = Options::Create();
        G = read_graph("../Matrix/bcspwr10.mtx");
        
        THEN( "The graph should not be null" )
        {
            REQUIRE( G != NULL );
        }
        AND_THEN( "The default options should not be null" )
        {
            REQUIRE( options != NULL);
        }
        AND_THEN ( "The number of vertices should be 5300" )
        {
            CHECK( G->n  == 5300 );
        }
        THEN ( "The number of edges should be 16542" )
        {
            CHECK( G->nz == 16542 );
        }

        WHEN( "an edge separator is computed with default options" )
        {
            ComputeEdgeSeparator(G, options);

            THEN( "the graph sizes should not change" )
            {
                REQUIRE( G->n  == 5300 );
                REQUIRE( G->nz == 16542 );
            }
            
            AND_THEN( "the graph should be partitioned" )
            {
                REQUIRE( G->partition != NULL );

                for (int i = 0; i < G->n; i++)
                {
                    bool equals_0 = G->partition[i] == 0;
                    bool equals_1 = G->partition[i] == 1;
                    CHECK( equals_0 + equals_1 == 1 );
                }
            }
        }
    }
}