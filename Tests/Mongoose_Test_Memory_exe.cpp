#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"

using namespace Mongoose;

int main(int argn, const char **argv)
{
    SuiteSparse_start();
    Logger::setDebugLevel(All);
    runMemoryTests();
    SuiteSparse_finish();
    
    return 0;
}