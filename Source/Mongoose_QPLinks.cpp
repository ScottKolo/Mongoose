
#include "Mongoose_QPLinks.hpp"
#include "Mongoose_QPBoundary.hpp"

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
    Weight *Ex = G->x;
    Weight *a = G->w;

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

    Double s = MONGOOSE_ZERO;

    for (Int k = 0; k < n; k++)
    {
        grad[k] = (0.5-x[k]) * D[k];
    }

    for (Int k = 0; k < n; k++)
    {
        Double xk = x[k];
        s += a[k] * xk;
        Weight r = 0.5 - xk;
        for (Int p = Ep[k]; p < Ep[k+1]; p++)
        {
            grad[Ei[p]] += r * Ex[p];
        }

        if (xk >= MONGOOSE_ONE)
        {
            FreeSet_status[k] = 1 ;
        }
        else if (xk <= MONGOOSE_ZERO)
        {
            FreeSet_status[k] = -1 ;
        }
        else
        {
            // add k to the FreeSet
#if FREESET_DEBUG
            printf ("Links: add k = %ld to the FreeSet\n", k) ;
#endif
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

    FreeSet_dump ("QPLinks:done", n, LinkUp, LinkDn, nFreeSet,
        FreeSet_status, 1) ;

    Double lo = G->W *
                (O->targetSplit <= 0.5 ? O->targetSplit : 1 - O->targetSplit);
    Double hi = G->W *
                (O->targetSplit >= 0.5 ? O->targetSplit : 1 - O->targetSplit);
    QP->ib = (s <= lo ? -1 : s < hi ? 0 : 1);
}

} // end namespace Mongoose
