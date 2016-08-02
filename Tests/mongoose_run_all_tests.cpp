
#include "mongoose.hpp"
#include "mongoose_internal.hpp"
#include "mongoose_io.hpp"
#include <string>
#include <iostream>
#include "mongoose_test.hpp"

using namespace Mongoose;

int main(int argn, const char **argv)
{
    run_memory_tests();
    run_io_tests();
    
    return 0;
}