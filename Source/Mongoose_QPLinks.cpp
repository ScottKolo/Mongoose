
#include "Mongoose_Internal.hpp"
#include "Mongoose_QPLinks.hpp"
#include "Mongoose_QPBoundary.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

void QPlinks
(
    Graph *G,
    Options *O,
    QPDelta *QP          /* pointer to QPDelta structure  */
)
{
    /* Inputs */
    Double *x = QP->x;

    /* Unpack structures. */
    Int n = G->n;
    Int *Ep = G->p;
    Int *Ei = G->i;
    Weight *Ex = G->x;      // TODO allow Ex NULL (means all 1s)
    Weight *a = G->w;       // TODO allow a NULL (means all 1s)

    /* working array */
    Double *D = QP->D;
    Int *FreeSet_status = QP->FreeSet_status;
    Int *LinkUp = QP->LinkUp;
    Int *LinkDn = QP->LinkDn;
    Double *grad = QP->gradient;  /* gradient at current x */

    Int lastl = n;

    // FreeSet is empty
    Int nFreeSet = 0;
    LinkUp[n] = n;
    LinkDn[n] = n;

    Double s = 0.;

    for (Int k = 0; k < n; k++)
    {
        grad[k] = (0.5-x[k]) * D[k];
    }

    for (Int k = 0; k < n; k++)
    {
        Double xk = x[k];
        if (xk < 0. || xk > 1.)
        {
            // TODO return an error condition here
        }
        // TODO do we also check a [k] > 0 ?
        s += a[k] * xk;
        Weight r = 0.5 - xk;
        for (Int p = Ep[k]; p < Ep[k+1]; p++)
        {
            grad[Ei[p]] += r * Ex[p];   // TODO allow Ex NULL (all 1s)
        }
        if (xk >= 1.)
        {
            FreeSet_status[k] = 1 ;
        }
        else if (xk <= 0.)
        {
            FreeSet_status[k] = -1 ;
        }
        else
        {
            // add k to the FreeSet
            FreeSet_status[k] = 0;
            LinkUp[lastl] = k;
            LinkDn[k] = lastl;
            lastl = k;
            nFreeSet++;
            //---
        }
    }

    LinkUp[lastl] = n;
    LinkDn[n] = lastl;
    QP->nFreeSet = nFreeSet;
    QP->b = s;

    DEBUG (FreeSet_dump ("QPLinks:done",
        n, LinkUp, LinkDn, nFreeSet, FreeSet_status, 1, x)) ;

    // make sure lo <= b <= hi holds, where b = a'*x and x is the input guess
    // TODO call napsack instead?
    if (QP->b >= QP->hi)
    {
        // b starts at the upper bound.
        // adjust upper bound to ensure x is feasible
        PR (("adjust hi from %g to b = %g\n", QP->hi, QP->b)) ;
        QP->hi = QP->b ;
        QP->ib = +1 ;
    }
    else if (QP->b <= QP->lo)
    {
        // b starts at the lower bound.
        // adjust lower bound to ensure x is feasible
        PR (("adjust lo from %g to b = %g\n", QP->lo, QP->b)) ;
        QP->lo = QP->b ;
        QP->ib = -1 ;
    }
    else
    {
        // b starts between: lo < b < hi
        QP->ib = 0 ;
    }

    // for debugging only
    QP->check_cost = INFINITY ;

    // ib is shorthand for these tests:
    Int ib = QP->ib ;
    PR (("QPlinks: target "
        "%g GW %g ib %ld lo %g b %g hi %g b-lo %g hi-b %g\n",
        O->targetSplit, G->W, ib, QP->lo, QP->b, QP->hi,
        (QP->b)-(QP->lo), (QP->hi)-(QP->b))) ;
    fflush (stdout) ;
    fflush (stderr) ;
    ASSERT (IMPLIES ((ib == -1), (QP->b == QP->lo))) ;          // b = lo
    ASSERT ((ib == 0) == (QP->lo < QP->b && QP->b < QP->hi)) ;  // lo < b <hi
    ASSERT (IMPLIES ((ib == +1), (QP->b == QP->hi))) ;          // b = hi
    ASSERT ((QP->lo <= QP->b && QP->b <= QP->hi)) ;             // x feasible
}

} // end namespace Mongoose
