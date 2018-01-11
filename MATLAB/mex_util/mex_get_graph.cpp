#include "mongoose_mex.hpp"

namespace Mongoose
{

Graph *mex_get_graph
(
    const mxArray *Gmatlab, /* The sparse matrix            */
    const mxArray *Amatlab  /* The real-valued node weights */
)
{
    // Check for valid sparse matrix
    cs_mex_check (0, -1, -1, 1, 1, 1, Gmatlab) ;

    Int n = mxGetN(Gmatlab);
    Int *Gp = (Int*) mxGetJc(Gmatlab);
    Int *Gi = (Int*) mxGetIr(Gmatlab);
    double *Gx =  (double*) mxGetPr(Gmatlab);
    Int nz = Gp[n];
    
    Graph *returner = Graph::Create(n, nz);

    if (!returner)
    {
        return NULL;
    }

    returner->p = Gp;
    returner->i = Gi;
    returner->x = Gx;

    /* Read node weights from matlab into the problem. */
    if (Amatlab != NULL)
    {
        SuiteSparse_free(returner->w);
        returner->w = (double*) mxGetPr(Amatlab);
    }
    else
    {
        // Assume node weights of 1
        for(Int k = 0; k < n; k++)
        {
            returner->w[k] = 1.0;
        }
    }

    return returner;
}

}
