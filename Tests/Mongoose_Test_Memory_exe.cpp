#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"

using namespace Mongoose;

int main(int argn, const char **argv)
{
    SuiteSparse_start();

    if (argn != 2)
    {
        // Wrong number of arguments - return error
        SuiteSparse_finish();
        return 1;
    }

    // Read in input file name
    std::string inputFile = std::string(argv[1]);

    // Set Logger to report only Test-level messages
    Logger::setDebugLevel(Test);

    // Run the memory test
    runMemoryTest(inputFile);

    SuiteSparse_finish();
    
    return 0;
}