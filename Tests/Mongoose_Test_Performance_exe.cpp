#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"

using namespace Mongoose;

int main(int argn, const char **argv)
{
    SuiteSparse_start();

    if (argn < 2 || argn > 3)
    {
        // Wrong number of arguments - return error
        SuiteSparse_finish();
        return 1;
    }

    // Read in input file name
    std::string inputFile = std::string(argv[1]);

    // Read in output file name
    std::string outputFile;
    if (argn == 3)
    {
        outputFile = std::string(argv[2]);
    }

    // Set Logger to report only Test and Error messages
    Logger::setDebugLevel(Test + Error);
    
    // Turn timing information on
    Logger::setTimingFlag(true);

    // Run the Edge Separator test
    runPerformanceTest(inputFile, outputFile);

    SuiteSparse_finish();
    
    return 0;
}