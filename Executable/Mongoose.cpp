
#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_Conditioning.hpp"
#include "Mongoose_CSparse.hpp"
#include "Mongoose_IO.hpp"
#include <ctime>

using namespace Mongoose;

int main(int argn, const char **argv)
{
    // Start the overall time clock
    std::clock_t start = std::clock();
    double duration;

    // Get the input file from the console input.
    const char *inputFile = NULL ;
    if (argn == 2)
        inputFile = argv[1] ;
    else
    {
        printf ("Usage: Demo <MM-input-file.mtx>\n") ;
        return 0 ;
    }

    Options *options = Options::Create();
    if (!options) return 1 ; // Return an error if we failed.

    options->doExpensiveChecks = false ;
    options->matchingStrategy = HEMDavisPA ;
    options->guessCutType = QP_BallOpt ;

    Graph *U = readGraph(inputFile);
    if (!U)
    {
        free(options);
        return 1;
    }

    ComputeEdgeSeparator (U, options) ;

    printf ("Partitioning Complete!\n") ;
    printf ("Cut Cost:      %.2f\n", U->cutCost) ;
    printf ("Cut Imbalance: %.2f%%\n", 100*U->imbalance) ;
    
    U->~Graph ();
    free(U);
    free(options);

    duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    printf("Total Time:    %.2fs\n", duration);

    // Return success
    return 0 ;
}
