#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_IO.hpp"
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
    Graph G1 = Graph();
    assert(&G1 != NULL);

    // Test Graph(n, nz) constructor
    Graph *G2 = Graph::Create(10, 20);
    assert(&G2 != NULL);

    SuiteSparse_finish();

    return 0;
}