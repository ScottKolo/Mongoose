#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"
#include <string>
#include <stdlib.h>

using namespace Mongoose;

int main(int argn, char** argv)
{
    SuiteSparse_start();

    // Set Logger to report all messages
    Logger::setDebugLevel(All);
    
    if (argn != 3)
    {
        // Wrong number of arguments - return error
        Logger::error() << "Usage: mongoose_test_io <MM-input-file.mtx> <1 for valid graph, 0 for invalid>\n";
        SuiteSparse_finish();
        return 1;
    }

    // Read in input file name
    std::string inputFile = std::string(argv[1]);

    // Read in whether this file should produce a valid graph
    bool validGraph = atoi(argv[2]);

    // Run the I/O test
    runIOTest(inputFile, validGraph);

    SuiteSparse_finish();

    return 0;
}