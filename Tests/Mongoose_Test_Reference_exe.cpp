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

    // Set Logger to report only Test and Error messages
    Logger::setDebugLevel(Test + Error);
    
    // Turn timing information on
    Logger::setTimingFlag(true);

    // Run the Reference performance test
    runReferenceTest(inputFile);

    SuiteSparse_finish();
    
    return 0;
}