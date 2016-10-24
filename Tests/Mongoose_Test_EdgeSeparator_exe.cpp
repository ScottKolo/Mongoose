#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"

using namespace Mongoose;

int main(int argc, char** argv)
{
    SuiteSparse_start();
    Logger::setDebugLevel(All);
    runEdgeSeparatorTests();
    SuiteSparse_finish();

    return 0;
}