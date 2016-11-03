#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_IO.hpp"
#include "Mongoose_Interop.hpp"
#include "Mongoose_CSparse.hpp"
#include <string>
#include <stdlib.h>

using namespace Mongoose;

int main(int argn, char** argv)
{
    SuiteSparse_start();

    // Set Logger to report all messages and turn off timing info
    Logger::setDebugLevel(All);
    Logger::setTimingFlag(false);

    // Test default constructor
    Graph G1;
    assert(&G1 != NULL);

    // Test Graph(n, nz) static constructor
    Graph *G2 = Graph::Create(10, 20);
    assert(&G2 != NULL);

    cs *M1 = GraphToCSparse3(G2, false);
    assert(M1 != NULL);
    cs *M2 = GraphToCSparse3(G2, true);
    assert(M2 != NULL);

    SuiteSparse_free(M1);
    M2->x = NULL;
    M2->p = NULL;
    M2->i = NULL;
    SuiteSparse_free(M2);

    SuiteSparse_finish();

    return 0;
}