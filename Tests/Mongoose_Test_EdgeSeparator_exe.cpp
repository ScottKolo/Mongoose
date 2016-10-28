#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"

using namespace Mongoose;

int main(int argc, char** argv)
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

    // Run the Edge Separator test
    runEdgeSeparatorTest();

    SuiteSparse_finish();

    return 0;
}