#include "Mongoose_Test.hpp"

int main(int argn, const char **argv)
{
    run_memory_tests();
    run_io_tests();
    
    return 0;
}