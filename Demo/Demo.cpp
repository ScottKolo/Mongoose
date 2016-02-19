
#include "mongoose.hpp"
#include "mongoose_conditioning.hpp"
#include "mongoose_cs.hpp"
#include <ctime>

using namespace SuiteSparse_Mongoose;

int main(int argn, const char **argv)
{
    std::clock_t start;
    double duration;

    start = std::clock();
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

    options->doExpensiveChecks = false ;
    options->matchingStrategy = HEMDavisPA ;
    options->guessCutType = QP_BallOpt ;

    /* Read & condition the matrix market input. */
    Graph *U = conditionGraph (readGraphFromMM (inputFile), options) ;
    /* Only process the largest connected component */
    cs *B = GraphToCSparse3(U, 1);
    csd* dmperm = cs_scc (B) ;
    int largest_size = 0 ;
    int largest_scc = 0 ;
    int scc_size = 0 ;
    for (int i = 0 ; i < dmperm->nb ; i++)
    {
        scc_size = dmperm->r[i+1]-1 - dmperm->r[i] ;
        if (scc_size > largest_size)
        {
            largest_size = scc_size ;
            largest_scc = i ;
        }
    }

    // Get submatrix from dmperm
    csi *pinv = cs_pinv(dmperm->p, B->n);
    cs *C = cs_permute(B, pinv, dmperm->p, 1);
    cs *submatrix = cs_submat(C, dmperm->r[largest_scc], 
                                dmperm->r[largest_scc+1]-1, 
                                dmperm->r[largest_scc], 
                                dmperm->r[largest_scc+1]-1) ;
    Graph *V = CSparse3ToGraph(submatrix);
    U = conditionGraph (V, options) ;
    ComputeEdgeSeparator (U, options) ;

    printf ("U->cutCost = %f\n", U->cutCost) ;
    printf ("U->imbalance = %f\n", U->imbalance) ;
    printf ("Partitioning Complete\n") ;

    U->~Graph () ;
    free (U) ;

    options->~Options () ;
    free (options) ;

    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

    printf("Total Time: %f\n", duration);

    /* Return success */
    return 0 ;
}
