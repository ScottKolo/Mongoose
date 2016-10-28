#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"
#include <string>
#include <stdlib.h>

using namespace Mongoose;

int main(int argn, char** argv)
{
    SuiteSparse_start();

    if (argn != 3)
    {
        // Wrong number of arguments - return error
        SuiteSparse_finish();
        return 1;
    }

    // Read in input file name
    std::string inputFile = std::string(argv[1]);

    // Read in whether this file should produce a valid graph
    bool validGraph = atoi(argv[2]);

    // Set Logger to report all messages
    Logger::setDebugLevel(All);

    // Run the I/O test
    runIOTest(inputFile, validGraph);

    SuiteSparse_finish();

    return 0;
}