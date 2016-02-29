
#include "mongoose.hpp"
#include "mongoose_conditioning.hpp"
#include "mongoose_cs.hpp"
#include <ctime>

#include "metis.h"

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
    printf("U->n = %ld\n", U->n);
    //ComputeEdgeSeparator (U, options) ;

    //printf ("U->cutCost = %f\n", U->cutCost) ;
    //printf ("U->imbalance = %f\n", U->imbalance) ;
    //printf ("Partitioning Complete\n") ;
    options->coarsenLimit = 50; // Performance problems at larger limits
    ComputeVertexSeparator (U, options);
    printf("Done!\n");

    U->~Graph () ;
    free (U) ;

    options->~Options () ;
    free (options) ;

    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    /*
    idx_t *perm = (idx_t*)SuiteSparse_malloc(U->n, sizeof(idx_t));
    idx_t *iperm = (idx_t*)SuiteSparse_malloc(U->n, sizeof(idx_t));
    idx_t met_options[METIS_NOPTIONS];
    METIS_SetDefaultOptions(met_options);
    met_options[METIS_OPTION_DBGLVL] = 1 + 2 + 4 + 8 + 64;
    met_options[METIS_OPTION_NO2HOP] = 0; 
    met_options[METIS_OPTION_CCORDER] = 1; 
    //int status = METIS_NodeND((idx_t*)U->n, (idx_t*)U->p, (idx_t*)U->i, (idx_t*)U->w, met_options, perm, iperm);
    */
    printf("Total Time: %f\n", duration);

    /* Return success */
    return 0 ;
}
