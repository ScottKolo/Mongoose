/* ========================================================================== */
/* === QPboundary =========================================================== */
/* ========================================================================== */

/*
   Move all components of x to boundary of the feasible region

    0 <= x <= 1, a'x = b, lo <= b <= hi

   while decreasing the cost function. The algorithm has the following parts

   1. For each i in the free set, see if x_i can be feasibly pushed to either
      boundary while decreasing the cost.

   2. For each i in the bound set, see if x_i can be feasibly flipped to
      opposite boundary while decreasing the cost.

   3. For each i in the free list with a_{ij} = 0 and with j free,
      move either x_i or x_j to the boundary while decreasing
      the cost. The adjustments has the form x_i = s/a_i and x_j = -s/a_j
      where s is a scalar factor. These adjustments must decrease cost.

   4. For the remaining i in the free list, take pair x_i and x_j and
      apply adjustments of the same form as in #2 above to push at least one
      component to boundary. The quadratic terms can only decrease the
      cost function. We choose the sign of s such that g_i x_i + g_j x_j <= 0.
      Hence, this adjustment cannot increase the cost.
 */

/* ========================================================================== */

#include "Mongoose_QPBoundary.hpp"
#include "Mongoose_source_internal.hpp"

namespace Mongoose
{

// TODO: move this function into a separate file with all FreeSet functions
void FreeSet_dump (const char *where,
    Int n, Int *LinkUp, Int *LinkDn, Int nFreeSet, Int *FreeSet_status,
    Int ix_sanity)
{
#if FREESET_DEBUG
    Int j ;
    Int death = 0 ;
    printf ("\ndump FreeSet (%s): nFreeSet %ld n %ld jfirst %ld\n",
        where, nFreeSet, n, LinkUp [n]) ;
    for (Int j = LinkUp[n]; j < n ; j = LinkUp[j])
    {
//      printf ("    j %3ld LinkUp[j] %3ld  LinkDn[j] %3ld ",
//          j, LinkUp [j], LinkDn [j]) ;
//      printf (" FreeSet_status %3ld\n", FreeSet_status [j]) ;
        death++ ;
        if (death > (nFreeSet+5)) assert (0) ;
    }
    printf ("    in FreeSet: %ld %ld\n", death, nFreeSet) ;
    assert (death == nFreeSet) ;

    Int nFree2 = 0 ;
    Int nHi = 0 ;
    Int nLo = 0 ;
    for (j = 0 ; j < n ; j++)
    {
        if (FreeSet_status [j] == 0)  nFree2++ ;
        else if (FreeSet_status [j] == 1)  nHi++ ;
        else if (FreeSet_status [j] == -1) nLo++ ;
        else assert (0) ;
    }
    printf ("    # that have FreeSet_status of zero: %ld\n", nFree2) ;
    printf ("    # that have FreeSet_status of one:  %ld\n", nHi) ;
    printf ("    # that have FreeSet_status of -1    %ld\n", nLo) ;
    if (nFreeSet != nFree2)
    {
        printf ("note nFree2 (%ld) and FreeSet_status[..]==0 ", nFree2) ;
        printf ("count (%ld) do not match.  sanity: %ld\n",
            nFreeSet, ix_sanity) ;
        if (ix_sanity) assert (nFreeSet == nFree2) ;
    }
#endif
}

void QPboundary
(
    Graph *G,
    Options *O,
    QPDelta *QP
)
{
    /* ---------------------------------------------------------------------- */
    /* Step 0. read in the needed arrays                                      */
    /* ---------------------------------------------------------------------- */

    /* input and output */

    //--- FreeSet
    Int nFreeSet = QP->nFreeSet;
    Int *LinkUp = QP->LinkUp;   /* linked list for free indices */
    Int *LinkDn = QP->LinkDn;   /* linked list, LinkDn [LinkUp [i] = i */
    Int *FreeSet_status = QP->FreeSet_status; 
        /* FreeSet_status [i] = +1, -1, or 0 
           if x_i = 1, 0, or 0 < x_i < 1 */
    //---

    if (nFreeSet == 0)
    {
        // quick return if FreeSet is empty
        return ;
    }

    Double pert0 = 0.5 - 1e-10; // QP->Parm->boundary_pert0;
    Double pert1 = 0.5 + 1e-10; // QP->Parm->boundary_pert1;
    Double *x = QP->x;          /* current estimate of solution */
    Double *grad = QP->gradient;       /* gradient at current x */
    Int ib = QP->ib;            /* ib = +1, -1, or 0 
                                   if b = hi, lo, or lo < b < hi */
    Double b = QP->b;           /* current value for a'x */

    /* problem specification for the graph G */
    Int n  = G->n;              /* problem dimension */
    Double *Ex = G->x;          /* numerical values for edge weights */
    Int *Ei = G->i;             /* adjacent vertices for each node */
    Int *Ep = G->p;             /* points into Ex or Ei */
    Double *a  = G->w;          /* a'x = b, lo <= b <= hi */
    Double lo = G->W *
                (O->targetSplit <= 0.5 ? O->targetSplit : 1 - O->targetSplit);
    Double hi = G->W *
                (O->targetSplit >= 0.5 ? O->targetSplit : 1 - O->targetSplit);
    Int *mark = G->mark;
    Int markValue = G->markValue;

    /* work array */
    Double *D  = QP->D;    /* diagonal of quadratic */

    /* ---------------------------------------------------------------------- */
    /* Step 1. if lo < b < hi, then for each free k,                          */
    /*         see if x_k can be pushed to 0 or 1                             */
    /* ---------------------------------------------------------------------- */

    FreeSet_dump ("step 1", n, LinkUp, LinkDn, nFreeSet, FreeSet_status, 1) ;

    // for each k in the FreeSet, but exit the loop when ib becomes nonzero:
    for (Int k = LinkUp[n]; (k < n) && (ib == 0) ; k = LinkUp[k])
    {
        Double s, ak = a[k];
        Int new_FreeSet_status_k = 0 ;
        if (grad[k] > 0.0) /* decrease x_k */
        {
            s = (b - lo) / ak;
            if (s < x[k])  /* lo is reached */
            {
                ib = -1;
                b = lo;
                x[k] -= s;
            }
            else          /* lo not reached */
            {
                s = x[k];
                x[k] = 0.;
                new_FreeSet_status_k = -1;
            }
        }
        else /* increase x_k */
        {
            s = (b - hi) / ak;
            if (s < x[k] - 1.) /* hi not reached */
            {
                s = x[k] - 1.;
                x[k] = 1.;
                new_FreeSet_status_k = +1;
            }
            else /* hi is reached */
            {
                ib = +1;
                b = hi;
                x[k] -= s;
            }
        }

        if (ib == 0) /* x_k hits boundary */
        {
            //--- remove k from FreeSet and update its status
#if FREESET_DEBUG
            printf ("(b0):remove k = %ld to the FreeSet\n", k) ;
            assert (FreeSet_status [k] == 0) ;
#endif
            FreeSet_status[k] = new_FreeSet_status_k ;
#if FREESET_DEBUG
            assert (FreeSet_status [k] != 0) ;
#endif
            nFreeSet--;
            Int h = LinkUp[k];
            Int g = LinkDn[k];
            LinkUp[g] = h;
            LinkDn[h] = g;
            FreeSet_dump ("QPBoundary:0", n, LinkUp, LinkDn,
                nFreeSet, FreeSet_status, 1) ;
            //---
            b -= s * ak;
        }

        for (Int p = Ep[k]; p < Ep[k+1]; p++)
        {
            grad[Ei[p]] += s * Ex[p];
        }

        grad[k] += s * D[k];
    }

    /* ---------------------------------------------------------------------- */
    /* Step 2. Examine flips of x_k from 0 to 1 or from 1 to 0 */
    /* ---------------------------------------------------------------------- */

    for (Int k = 0; k < n; k++)
    {
        Int FreeSet_status_k = FreeSet_status[k];
        if (FreeSet_status_k == 0)
        {
            // k is in FreeSet so it cannot be simply flipped 0->1 or 1->0
            continue;
        }

        // k not in FreeSet, so no changes here to LinkUp, LinkDn, or nFreeSet

        Double ak = a[k];
        if (FreeSet_status_k > 0) /* try changing x_k from 1 to 0 */
        {
            if (b - ak >= lo)
            {
                if (0.5 * D[k] + grad[k] >= 0) /* flip lowers cost */
                {
                    b -= ak;
                    ib = (b == lo ? -1 : 0);
                    x[k] = MONGOOSE_ZERO;
                    FreeSet_status[k] = -1;
                }
            }
        }
        else /* try changing x_k from 0 to 1 */
        {
            if (b + ak <= hi)
            {
                if (grad[k] - 0.5 * D[k] <= 0) /* flip lowers cost */
                {
                    b += ak;
                    ib = (b == hi ? 1 : 0);
                    x[k] = MONGOOSE_ONE;
                    FreeSet_status[k] = +1;
                }
            }
        }

        if (FreeSet_status_k != FreeSet_status[k])
        {
            if (FreeSet_status_k == 1) /* x [k] was 1, now it is 0 */
            {
                for (Int p = Ep[k]; p < Ep[k+1]; p++)
                {
                    grad[Ei[p]] += Ex[p];
                }
                grad[k] += D[k];
            }
            else /* x [k] was 0, now it is 1 */
            {
                for (Int p = Ep[k]; p < Ep[k+1]; p++)
                {
                    grad[Ei[p]] -= Ex[p];
                }
                grad[k] -= D[k];
            }
        }
    }

    /* ---------------------------------------------------------------------- */
    // quick return if FreeSet is now empty
    /* ---------------------------------------------------------------------- */

    if (nFreeSet == 0)
    {
        QP->nFreeSet = nFreeSet;
        QP->b = b;
        QP->ib = ib;
        return;
    }

    /* ---------------------------------------------------------------------- */
    /* Step 3. Search for a_{ij} = 0 in the free index set */
    /* ---------------------------------------------------------------------- */

    // look for where both i and j are in the FreeSet,
    // but i and j are not adjacent in the graph G.

    FreeSet_dump ("step 3", n, LinkUp, LinkDn, nFreeSet, FreeSet_status, 1) ;

    // for each j in FreeSet, and while |FreeSet| > 1:
    for (Int j = LinkUp[n]; (j < n) && (nFreeSet > 1) ; j = LinkUp[j])
    {
#if FREESET_DEBUG
        printf ("j %g nFreeSet %g\n", (double) j, (double) nFreeSet) ;
#endif

        // TODO merge this loop with the next one.  Clear marks if continue.
        // count how many neighbors of j are free
        Int m = 1;                                  // (including j itself)
        for (Int p = Ep[j]; p < Ep[j+1]; p++)
        {
            Int i = Ei[p] ;
            assert (i != j) ;                       // graph has no self edges
            if (FreeSet_status[i] == 0) m++;
        }
        // do not consider j if all its neighbors are in the FreeSet
        if (m == nFreeSet) continue;

        /* -------------------------------------------------------------- */
        /* otherwise there exist i and j free with a_{ij} = 0, scatter Ei */
        /* -------------------------------------------------------------- */

        // mark all nodes i adjacent to j in the FreeSet
        for (Int p = Ep[j]; p < Ep[j+1]; p++)
        {
            Int i = Ei[p] ;
            assert (i != j) ;                       // graph has no self edges
            MONGOOSE_MARK(i);
        }
        MONGOOSE_MARK(j);

        // for each i in FreeSet:
        for (Int i = LinkUp[n]; i < n ; i = LinkUp[i])
        {
            if (!MONGOOSE_MARKED(i))
            {
                // node i is not adjacent to j in the graph G 
                Double aj = a[j];
                Double ai = a[i];
                Double xi = x[i];
                Double xj = x[j];

                /* cost change if x_j increases dx_j = s/a_j, dx_i = s/a_i */
                Double s;
                Int bind1, bind2;
                if (aj * (MONGOOSE_ONE - xj) < ai * xi) // x_j hits upper bound
                {
                    s = aj * (MONGOOSE_ONE - xj);
                    bind1 = 1;
                }
                else /* x_i hits lower bound */
                {
                    s = ai * xi;
                    bind1 = 0;
                }
                Double dxj = s / aj;
                Double dxi = -s / ai;
                Double c1 = (grad[j] - .5 * D[j] * dxj) * dxj +
                            (grad[i] - .5 * D[i] * dxi) * dxi;

                /* cost change if x_j decreases dx_j = s/a_j, dx_i = s/a_i */
                if (aj * xj < ai * (MONGOOSE_ONE - xi)) // x_j hits lower bound
                {
                    s = -aj * xj;
                    bind2 = -1;
                }
                else /* x_i hits upper bound */
                {
                    s = -ai * (MONGOOSE_ONE - xi);
                    bind2 = 0;
                }
                dxj = s / aj;
                dxi = -s / ai;
                Double c2 = (grad[j] - 0.5 * D[j] * dxj) * dxj +
                            (grad[i] - 0.5 * D[i] * dxi) * dxi;

                Int new_FreeSet_status ;
                if (c1 < c2) /* increase x_j */
                {
                    if (bind1 == 1)
                    {
                        // j is bound (not i) and x_j becomes 1
                        dxj = 1. - xj;
                        dxi = -aj * dxj / ai;
                        x[j] = 1.;
                        x[i] += dxi;
                        new_FreeSet_status = +1; /* j is bound at 1 */
                    }
                    else // bind1 is zero
                    {
                        // i is bound (not j) and x_i becomes 0
                        dxi = -xi;
                        dxj = -ai * dxi / aj;
                        x[i] = 0.;
                        x[j] += dxj;
                        new_FreeSet_status = -1; /* i is bound at 0 */
                    }
                }
                else
                {
                    if (bind2 == -1)
                    {
                        // j is bound (not i) and x_j becomes 0
                        bind1 = 1;
                        x[j] = 0.;
                        x[i] += dxi;
                        new_FreeSet_status = -1; /* j is bound at 0 */
                    }
                    else /* x_i = 1 */
                    {
                        // i is bound (not j) and x_i becomes 1
                        bind1 = 0;
                        x[i] = 1;
                        x[j] += dxj;
                        new_FreeSet_status = +1; /* i is bound at 1 */
                    }
                }

                for (Int p = Ep[j]; p < Ep[j+1]; p++)
                {
                    grad[Ei[p]] -= Ex[p] * dxj;
                }
                for (Int p = Ep[i]; p < Ep[i+1]; p++) 
                {
                    grad[Ei[p]] -= Ex[p] * dxi;
                }
                grad[j] -= D[j] * dxj;
                grad[i] -= D[i] * dxi;

                if (bind1)
                {
                    // remove j from the FreeSet
#if FREESET_DEBUG
                    printf ("(b1):remove j = %ld from the FreeSet\n", j) ;
#endif
                    FreeSet_dump ("QPBoundary:1 before", n, LinkUp, LinkDn,
                        nFreeSet, FreeSet_status, 1) ;
                    assert (FreeSet_status [j] == 0) ;
                    FreeSet_status [j] = new_FreeSet_status ;
                    assert (FreeSet_status [j] != 0) ;
                    nFreeSet--;
                    Int h = LinkUp[j];
                    Int g = LinkDn[j];
                    LinkUp[g] = h;
                    LinkDn[h] = g;
                    FreeSet_dump ("QPBoundary:1", n, LinkUp, LinkDn,
                        nFreeSet, FreeSet_status, 1) ;
                    //---
                    break;
                }
                else
                {
                    // remove i from the FreeSet
#if FREESET_DEBUG
                    printf ("(b2):remove i = %ld from the FreeSet\n", i) ;
#endif
                    FreeSet_dump ("QPBoundary:2 before", n, LinkUp, LinkDn,
                        nFreeSet, FreeSet_status, 1) ;
                    assert (FreeSet_status [i] == 0) ;
                    FreeSet_status [i] = new_FreeSet_status ;
                    assert (FreeSet_status [i] != 0) ;
                    nFreeSet--;
                    Int h = LinkUp[i];
                    Int g = LinkDn[i];
                    LinkUp[g] = h;
                    LinkDn[h] = g;
                    FreeSet_dump ("QPBoundary:2", n, LinkUp, LinkDn,
                        nFreeSet, FreeSet_status, 1) ;
                    //---
                    continue;
                }
            }
        }

        // clear the marks from all the nodes
        MONGOOSE_CLEAR_ALL_MARKS ;
    }

    /* ---------------------------------------------------------------------- */
    // assert that the nodes in the FreeSet form a single clique
    /* ---------------------------------------------------------------------- */

    assert (nFreeSet >= 1) ;    // we can have 1 or more nodes still in FreeSet

    // TODO: only run this test in debug mode:
    for (Int j = LinkUp[n]; (j < n) ; j = LinkUp[j])
    {
        // j must be adjacent to all other nodes in the FreeSet
        Int nfree_neighbors = 0 ;
        for (Int p = Ep[j]; p < Ep[j+1]; p++)
        {
            Int i = Ei[p] ;
            assert (i != j) ;
            if (FreeSet_status [i] == 0) nfree_neighbors++ ;
        }
#if FREESET_DEBUG
        assert (nfree_neighbors == nFreeSet - 1) ;
#endif
    }

    /* ---------------------------------------------------------------------- */
    /* Step 4. dxj = s/aj, dxi = -s/ai, choose s with g_j dxj + g_i dxi <= 0 */
    /* ---------------------------------------------------------------------- */

    FreeSet_dump ("step 4", n, LinkUp, LinkDn, nFreeSet, FreeSet_status, 1) ;

    // for each j in the FreeSet:
    for (Int j = LinkUp [n] ; j < n ; /* see below for next j */ )
    {
        /* free variables: 0 < x_j < 1 */
        /* choose s so that first derivative terms decrease */

        // i is the next variable after j in the FreeSet
        Int i = LinkUp[j];
        if (i == n) break;  // TODO: if (i == EMPTY) break ;

        Double ai = a[i];
        Double aj = a[j];
        Double xi = x[i];
        Double xj = x[j];

        Int new_FreeSet_status ;
        Int bind1;
        Double dxj, dxi, s = grad[j] / aj - grad[i] / ai;

        if (s < MONGOOSE_ZERO) /* increase x_j */
        {
            if (aj * (MONGOOSE_ONE - xj) < ai * xi) /* x_j hits upper bound */
            {
                dxj = MONGOOSE_ONE - xj;
                dxi = -aj * dxj / ai;
                x[j] = MONGOOSE_ONE;
                x[i] += dxi;
                new_FreeSet_status = +1;
                bind1 = 1; /* x_j is bound at 1 */
            }
            else /* x_i hits lower bound */
            {
                dxi = -xi;
                dxj = -ai * dxi / aj;
                x[i] = 0.;
                x[j] += dxj;
                new_FreeSet_status = -1;
                bind1 = 0; /* x_i is bound at 0 */
            }
        }
        else /* decrease x_j */
        {
            if (aj * xj < ai * (1. - xi)) /* x_j hits lower bound */
            {
                dxj = -xj;
                dxi = -aj * dxj / ai;
                x[j] = 0;
                x[i] += dxi;
                new_FreeSet_status = -1;
                bind1 = 1; /* x_j is bound */
            }
            else /* x_i hits upper bound */
            {
                dxi = 1 - xi;
                dxj = -ai * dxi / aj;
                x[i] = 1;
                x[j] += dxj;
                new_FreeSet_status = +1;
                bind1 = 0; /* x_i is bound */
            }
        }

        for (Int k = Ep[j]; k < Ep[j+1]; k++)
        {
            grad[Ei[k]] -= Ex[k] * dxj;
        }
        for (Int k = Ep[i]; k < Ep[i+1]; k++)
        {
            grad[Ei[k]] -= Ex[k] * dxi;
        }
        grad[j] -= D[j] * dxj;
        grad[i] -= D[i] * dxi;

        // ---------------------------------------------------------------------
        // the following 2 cases define the next j in the iteration:
        // ---------------------------------------------------------------------

        if (bind1) /* j is bound */
        {
            // remove j from the FreeSet
#if FREESET_DEBUG
            printf ("(b3):remove j = %ld from the FreeSet\n", j) ;
#endif
            FreeSet_dump ("QPBoundary:3 before", n, LinkUp, LinkDn,
                nFreeSet, FreeSet_status, 1) ;
            assert (FreeSet_status [j] == 0) ;
            FreeSet_status [j] = new_FreeSet_status ;
            assert (FreeSet_status [j] != 0) ;
            nFreeSet--;
            Int h = LinkUp[j];
            Int g = LinkDn[j];
            LinkUp[g] = h;
            LinkDn[h] = g;
            FreeSet_dump ("QPBoundary:3", n, LinkUp, LinkDn,
                nFreeSet, FreeSet_status, 1) ;
            //---
            // go to next j in the list
            j = LinkUp [j] ;
        }
        else /* i is bound */
        {
            // remove i from the FreeSet
            FreeSet_dump ("QPBoundary:4 before", n, LinkUp, LinkDn,
                nFreeSet, FreeSet_status, 1) ;
#if FREESET_DEBUG
            printf ("(b4):remove i = %ld from the FreeSet\n", i) ;
#endif
            assert (FreeSet_status [i] == 0) ;
            FreeSet_status [i] = new_FreeSet_status ;
            assert (FreeSet_status [i] != 0) ;
            nFreeSet--;
            Int h = LinkUp[i];
            Int g = LinkDn[i];
            LinkUp[g] = h;
            LinkDn[h] = g;
            FreeSet_dump ("QPBoundary:4", n, LinkUp, LinkDn,
                nFreeSet, FreeSet_status, 1) ;
            //---
            // j is still in the list and remains the same;
            // do not advance to the next in the list
        }
    }

    FreeSet_dump ("wrapup", n, LinkUp, LinkDn, nFreeSet, FreeSet_status, 1) ;

    /* ---------------------------------------------------------------------- */
    /* step 5: single free variable remaining */
    /* ---------------------------------------------------------------------- */

    if (nFreeSet == 1) /* j is free, optimize over x [j] */
    {
        // j is the first and only item in the FreeSet
        Int j = LinkUp [n] ;
#if FREESET_DEBUG
        printf ("ONE AND ONLY!! j = %ld\n", j) ;
#endif
        Int bind1 = 0;
        Double aj = a[j];
        Double dxj = (hi - b) / aj;
        if (dxj < MONGOOSE_ONE - x[j])
        {
            bind1 = 1;
        }
        else
        {
            dxj = MONGOOSE_ONE - x[j];
        }

        Int bind2 = 0;
        Double dxi = (lo - b) / aj;
        if (dxi > -x[j])
        {
            bind2 = 1;
        }
        else
        {
            dxi = -x[j];
        }

        Double c1 = (grad[j] - 0.5 * D[j] * dxj) * dxj;
        Double c2 = (grad[j] - 0.5 * D[j] * dxi) * dxi;
        if (c1 <= c2) /* x [j] += dxj */
        {
            if (bind1)
            {
                x[j] += dxj;
                ib = +1;
                b = hi;
            }
            else
            {
                /// remove j from the FreeSet, which is now empty
#if FREESET_DEBUG
                printf ("(b5):remove j = %ld from FreeSet, now empty\n", j) ;
#endif
                FreeSet_dump ("QPBoundary:5 before", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 1) ;
                assert (FreeSet_status [j] == 0) ;
                FreeSet_status[j] = 1;
                assert (FreeSet_status [j] != 0) ;
                nFreeSet--;
                LinkUp[n] = n;
                LinkDn[n] = n;
                FreeSet_dump ("QPBoundary:5", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 1) ;
                assert (nFreeSet == 0) ;
                //--- 
                x[j] = 1.;
                b += dxj * aj;
            }
        }
        else /* x [j] += dxi */
        {
            dxj = dxi;
            if (bind2)
            {
                x[j] += dxj;
                ib = -1;
                b = lo;
            }
            else
            {
                /// remove j from the FreeSet, which is now empty
#if FREESET_DEBUG
                printf ("(b6):remove j = %ld from FreeSet, now empty\n", j) ;
#endif
                FreeSet_dump ("QPBoundary:6 before", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 1) ;
                assert (FreeSet_status [j] == 0) ;
                FreeSet_status[j] = -1;
                assert (FreeSet_status [j] != 0) ;
                nFreeSet--;
                LinkUp[n] = n;
                LinkDn[n] = n;
                FreeSet_dump ("QPBoundary:6", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 1) ;
                assert (nFreeSet == 0) ;
                //---
                x[j] = 0.;
                b += dxj * aj;
            }
        }

        if (dxj != MONGOOSE_ZERO)
        {
            for (Int p = Ep[j]; p < Ep[j+1]; p++)
            {
                grad[Ei[p]] -= Ex[p] * dxj;
            }
            grad[j] -= D[j] * dxj;
        }

        // TODO I don't understand this section.  I added the test ("if
        // (nFreeSet == 1)").  But what if j had already been removed from the
        // free set in the tests above?  Without the "if" test, it could be
        // removed again.

        if (nFreeSet == 1)          // TODO. Added this test.  Is it OK?
        {
            if ((x[j] >= pert1) || ((aj == MONGOOSE_ONE) && (x[j] > 0.5)))
            {
                // remove j from the FreeSet, which is now empty
#if FREESET_DEBUG
                printf ("(b7):remove j = %ld from FreeSet, now empty\n", j) ;
#endif
                FreeSet_dump ("QPBoundary:7 before", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 1) ;
                assert (FreeSet_status [j] == 0) ;
                FreeSet_status[j] = +1;
                nFreeSet--;
                LinkUp[n] = n;
                LinkDn[n] = n;
                assert (nFreeSet == 0) ;
                FreeSet_dump ("QPBoundary:7", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 1) ;
                //---
                x[j] = 1.;
                // TODO: ask Hager why b is not updated
            }
            else if ((x[j] <= pert0) || ((aj == 1.) && (x[j] < .5)))
            {
                // remove j from the FreeSet, which is now empty
#if FREESET_DEBUG
                printf ("(b8):remove j = %ld from FreeSet, now empty\n", j) ;
#endif
                FreeSet_dump ("QPBoundary:8 before", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 1) ;
                assert (FreeSet_status [j] == 0) ;
                FreeSet_status[j] = -1;
                nFreeSet--;
                LinkUp[n] = n;
                LinkDn[n] = n;
                assert (nFreeSet == 0) ;
                FreeSet_dump ("QPBoundary:8", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 1) ;
                //---
                x[j] = 0.;
                // TODO: ask Hager why b is not updated
            }
        }
    }

    QP->nFreeSet = nFreeSet;
    QP->b = b;
    QP->ib = ib;

    // clear the marks from all the nodes
    MONGOOSE_CLEAR_ALL_MARKS ;      // TODO: reset if int overflow
    G->markValue = markValue ;

}

} // end namespace Mongoose
