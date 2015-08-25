
#include "mongoose.hpp"
#include "mongoose_conditioning.hpp"

using namespace SuiteSparse_Mongoose;

int main(int argn, const char **argv)
{
    /* Get the input file from the console input. */
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

    options->doExpensiveChecks = true ;
    options->matchingStrategy = HEMDavisPA ;
    options->guessCutType = QP_BallOpt ;

    /* Read & condition the matrix market input. */
    Graph *U = conditionGraph (readGraphFromMM (inputFile), options) ;

    ComputeEdgeSeparator (U, options) ;

    printf ("U->cutCost = %f\n", U->cutCost) ;
    printf ("U->imbalance = %f\n", U->imbalance) ;
    printf ("Partitioning Complete\n") ;

    U->~Graph () ;
    free (U) ;

    options->~Options () ;
    free (options) ;

    /* Return success */
    return 0 ;
}
