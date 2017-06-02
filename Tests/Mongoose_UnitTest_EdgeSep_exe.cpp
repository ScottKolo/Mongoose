
#define LOG_ERROR 1
#define LOG_WARN 1
#define LOG_INFO 0
#define LOG_TEST 1

#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_IO.hpp"
#include "Mongoose_Interop.hpp"
#include "Mongoose_EdgeSeparator.hpp"

using namespace Mongoose;

int main(int argn, char** argv)
{
    (void)argn; // Unused variable
    (void)argv; // Unused variable

    SuiteSparse_start();

    // Set Logger to report all messages and turn off timing info
    Logger::setDebugLevel(All);
    Logger::setTimingFlag(false);

    Graph *G = readGraph("../Matrix/bcspwr02.mtx");

    // Test with no options struct
    ComputeEdgeSeparator(G);

    // Test with NULL options struct
    Options *O = NULL;
    ComputeEdgeSeparator(G, O);

    // Test with invalid targetSplit
    O = Options::Create();
    O->targetSplit = 1.2;
    ComputeEdgeSeparator(G, O);
    O->targetSplit = 0.4;

    // Test with invalid tolerance
    O->softSplitTolerance = -1;
    ComputeEdgeSeparator(G, O);

    SuiteSparse_finish();

    return 0;
}
