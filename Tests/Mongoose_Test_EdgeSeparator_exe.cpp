#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"

using namespace Mongoose;

#define LOG_ERROR 1
#define LOG_WARN 1
#define LOG_INFO 0
#define LOG_TEST 1

int main(int argn, char** argv)
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

    // Set Logger to report all messages
    Logger::setDebugLevel(All);

    // Turn timing information on
    Logger::setTimingFlag(true);

    // Run the Edge Separator test
    runEdgeSeparatorTest(inputFile);

    SuiteSparse_finish();

    return 0;
}