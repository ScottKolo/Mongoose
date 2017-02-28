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

namespace Mongoose
{

//------------------------------------------------------------------------------
// QPcheckCom
//------------------------------------------------------------------------------

// Check that the QPcom data structure is consistent and cost decreases

#define ERROR { fflush (stdout) ; fflush (stderr) ; ASSERT (0) ; }

void QPcheckCom
(
    Graph *G,
    Options *O,
    QPDelta *QP,
    bool check_b,
    Int nFreeSet,       // use this instead of QP->nFreeSet
    Double b            // use this instead of QP->b
)
{
    Int i, j, k, l ;
    Double s, t ;

    //--- FreeSet
//  Int nFreeSet = QP->nFreeSet ;  /* number of i such that 0 < x_i < 1 */
    Int *LinkUp = QP->LinkUp ; /* linked list for free indices */
    Int *LinkDn = QP->LinkDn ; /* linked list, LinkDn [LinkUp [i] = i*/
    Int *FreeSet_status = QP->FreeSet_status ; 
        /* FreeSet_status [i] = +1, -1, or 0 if x_i = 1, 0, or 0 < x_i < 1*/
    //---

    Double *x = QP->x ;/* current estimate of solution */

    /* problem specification */
    Int n  = G->n ;  /* problem dimension */
    Double *Ex = G->x ; /* numerical values for edge weights */
    Int *Ei = G->i ; /* adjacent vertices for each node */
    Int *Ep = G->p ; /* points into Ex or Ei */
    Double *a  = G->w ;   /* a'x = b, lo <= b <= hi */

    Double lo = QP->lo ;
    Double hi = QP->hi ;
    Int *mark = G->mark;
    Int markValue = G->markValue;
    Double *D  = QP->D ;   /* diagonal of quadratic */
    Double *grad  = QP->gradient ;   /* gradient at current x */
    Double tol = 1e-8 ;

    // get workspace
    Int *w0 = (Int *) calloc (n+1, sizeof (Int)) ;    // [
    Double *gtemp = (Double *) malloc ((n+1) * sizeof (Double)) ;    // [

    /* check that lo <= hi */
    if ( lo > hi )
    {
        printf ("lo %e > hi %e\n", lo, hi) ;
        ERROR ;
    }

    /* check feasibility */
    if ( a == NULL )
    {
        // a is implicitly all 1's
        s = (Double) n ;
        t = 0. ;
    }
    else
    {
        s = 0. ;
        t = 0. ;
        for (j = 0; j < n; j++)
        {
            if ( a [j] > 0 ) s += a [j] ;
            else             t += a [j] ;
        }
    }

    if ( s < lo )
    {
        printf ("lo %e > largest possible value %e\n", lo, s) ;
        ERROR ;
    }
    if ( t > hi )
    {
        printf ("hi %e < smallest possible value %e\n", hi, t) ;
        ERROR ;
    }

    Int *ix = FreeSet_status ;

    /* check that nFreeSet = number of zeroes in ix, and ix agrees with x */
    i = 0 ;
    for (j = 0; j < n; j++)
    {
        if ( ix [j] > 1 )
        {
            printf ("ix [%ld] = %ld (> 1)\n", j, ix [j]) ;
            ERROR ;
        }
        else if (ix [j] < -1)
        {
            printf ("ix [%ld] = %ld (< -1)\n", j, ix [j]) ;
            ERROR ;
        }
        else if ( ix [j] == 0 ) i++ ;
        k = 0 ;
        if ( ix [j] == 1 )
        {
            if ( x [j] != 1. ) k = 1 ;
        }
        else if ( ix [j] == -1 )
        {
            if ( x [j] != 0. ) k = 1 ;
        }
        if ( k )
        {
            printf ("ix [%ld] = %ld while x = %e\n", j, ix [j], x [j]) ;
            ERROR ;
        }
        if ( (x [j] > 1+tol) || (x [j] < -tol) )
        {
            printf ("x [%ld] = %e outside range [0, 1]", j, x [j]) ;
            ERROR ;
        }
    }
    if ( i != nFreeSet )
    {
        printf ("free indices in ix is %ld, nFreeSet = %ld\n", i, nFreeSet) ;
        ERROR ;
    }

    /* check that LinkUp is valid */

    l = 0 ;
    for (i = LinkUp [n]; i < n; i = LinkUp [j])
    {
        if ( (i < 0) || (i > n) )
        {
            printf ("LinkUp [%ld] = %ld, out of range [0, %ld]\n", j, i, n) ;
            ERROR ;
        }
        if ( w0 [i] != 0 )
        {
            printf ("LinkUp [%ld] = %ld, repeats\n", j, i) ;
            ERROR ;
        }
        if ( ix [i] != 0 )
        {
            printf ("LinkUp [%ld] = %ld, however it is not free\n", j, i) ;
            ERROR ;
        }
        w0 [i] = 1 ;
        j = i ;
        l++ ;
    }

    if ( l != nFreeSet )
    {
        printf ("free indices in LinkUp is %ld, nFreeSet = %ld\n", i,
            nFreeSet) ;
        ERROR ;
    }

    if ( i != n )
    {
        printf ("LinkUp [%ld] = %ld, out of range [0, %ld]\n", j, i, n) ;
        ERROR ;
    }

    for (j = LinkUp [n]; j < n; j = LinkUp [j]) w0 [j] = 0 ;

    /* check that LinkDn is valid */

    j = n ;
    for (i = LinkUp [n]; i < n; i = LinkUp [j])
    {
        if ( LinkDn [i] != j )
        {
            printf ("LinkDn [%ld] = %ld (!= %ld)\n", i, LinkDn [i], j) ;
            ERROR ;
        }
        j = i ;
    }

    /* check that b is correct */

    s = 0. ;
    if ( a == NULL ) for (j = 0; j < n; j++) s += x [j] ;
    else             for (j = 0; j < n; j++) s += x [j]*a [j] ;
    // printf ("CHECK BOUNDS: lo %g s=a'x %g hi %g\n", lo, s, hi) ;
    if ( check_b )
    {
        if ( fabs (b-s) > tol )
        {
            printf ("QP->b = %e while a'x = %e\n", b, s) ;
            ERROR ;
        }
    }
    if ( s < lo - tol )
    {
        printf ("a'x TOO LO: a'x = %e < lo = %e\n", s, lo) ;
        ERROR ;
    }
    if ( s > hi + tol )
    {
        printf ("a'x TOO HI: a'x = %e > hi = %e\n", s, hi) ;
        ERROR ;
    }

    /* check that grad is correct */

    for (j = 0; j < n; j++) gtemp [j] = (.5-x [j])*D [j] ;
    Double newcost = 0. ;
    if ( Ex == NULL )
    {
        for (j = 0; j < n; j++)
        {
            s = .5 - x [j] ;
            t = 0. ;
            for (k = Ep [j]; k < Ep [j+1]; k++)
            {
                gtemp [Ei [k]] += s ;
                t += x [Ei [k]] ;
            }
            newcost += (t + x [j]*D [j])*(1.-x[j]) ;
        }
    }
    else
    {
        for (j = 0; j < n; j++)
        {
            s = .5 - x [j] ;
            t = 0. ;
            for (k = Ep [j]; k < Ep [j+1]; k++)
            {
                gtemp [Ei [k]] += s*Ex [k] ;
                t += Ex [k]*x [Ei [k]] ;
            }
            newcost += (t + x [j]*D [j])*(1.-x[j]) ;
        }
    }
    s = 0. ;
    for (j = 0; j < n; j++) s = MONGOOSE_MAX2 (s, fabs (gtemp [j]-grad [j])) ;
    if ( s > tol )
    {
        printf ("error (%e) in grad: current grad, true grad, x:\n", s) ;
        for (j = 0; j < n; j++)
        {
            double ack = fabs (gtemp [j] - grad [j]) ;
            printf ("j: %5ld grad: %15.6e gtemp: %15.6e err: %15.6e "
                "x: %15.6e", j, grad [j], gtemp [j], ack, x [j]) ;
            if (ack > tol) printf (" ACK!") ;
            printf ("\n") ;
        }
        ERROR ;
    }

    /* check that cost decreases */

    if ( newcost > tol + QP->check_cost )
    {
        printf ("cost increases, old %30.15e new %30.15e\n",
            QP->check_cost, newcost) ;
        ERROR ;
    }
    QP->check_cost = newcost ;
    printf ("cost: %30.15e\n", newcost) ;

    free (gtemp) ;  // ]
    free (w0) ;  // ]
}

//------------------------------------------------------------------------------
// FreeSet_dump
//------------------------------------------------------------------------------

// TODO: move this function into a separate file with all FreeSet functions
void FreeSet_dump (const char *where,
    Int n, Int *LinkUp, Int *LinkDn, Int nFreeSet, Int *FreeSet_status,
    Int verbose, Double *x)
{
#ifndef NDEBUG
    Int j ;
    Int death = 0 ;
    if (verbose)
    {
        printf ("\ndump FreeSet (%s): nFreeSet %ld n %ld jfirst %ld\n",
        where, nFreeSet, n, LinkUp [n]) ;
    }
    for (Int j = LinkUp[n]; j < n ; j = LinkUp[j])
    {
        if (verbose)
        {
            printf ("    j %3ld LinkUp[j] %3ld  LinkDn[j] %3ld ",
            j, LinkUp [j], LinkDn [j]) ;
            printf (" FreeSet_status %3ld", FreeSet_status [j]) ;
            if (x != NULL) printf (" x: %g", x [j]) ;
            printf ("\n") ;
        }
        death++ ;
        if (death > (nFreeSet+5)) ASSERT(0) ;
    }
    if (verbose) printf ("    in FreeSet: %ld %ld\n", death, nFreeSet) ;
    ASSERT(death == nFreeSet) ;

    Int nFree2 = 0 ;
    Int nHi = 0 ;
    Int nLo = 0 ;
    for (j = 0 ; j < n ; j++)
    {
        fflush (stdout) ;
        fflush (stderr) ;
        if (FreeSet_status [j] == 0)
        {
            // note that in rare cases, x[j] can be 0 or 1 yet
            // still be in the FreeSet.
            if (x != NULL) ASSERT (0 <= x [j] && x [j] <= 1.) ;
            nFree2++ ;
        }
        else if (FreeSet_status [j] == 1)
        {
            if (x != NULL) ASSERT (x [j] >= 1.) ;
            nHi++ ;
        }
        else if (FreeSet_status [j] == -1)
        {
            if (x != NULL) ASSERT (x [j] <= 0.) ;
            nLo++ ;
        }
        else
        {
            ASSERT(0) ;
        }
    }
    if (verbose)
    {
        printf ("    # that have FreeSet_status of zero: %ld\n", nFree2) ;
        printf ("    # that have FreeSet_status of one:  %ld\n", nHi) ;
        printf ("    # that have FreeSet_status of -1    %ld\n", nLo) ;
    }
    if (nFreeSet != nFree2)
    {
        printf ("ERROR nFree2 (%ld) nFreeSet %ld\n", nFree2, nFreeSet) ;
        ASSERT (0) ;
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

    Double lo = QP->lo ;
    Double hi = QP->hi ;
//  Double lo = G->W *
//              (O->targetSplit <= 0.5 ? O->targetSplit : 1 - O->targetSplit);
//  Double hi = G->W *
//              (O->targetSplit >= 0.5 ? O->targetSplit : 1 - O->targetSplit);

    Int *mark = G->mark;
    Int markValue = G->markValue;

    /* work array */
    Double *D  = QP->D;    /* diagonal of quadratic */

    printf ("\n----- QPboundary start: [\n") ;
    QPcheckCom (G, O, QP, 1, QP->nFreeSet, QP->b) ;      // check b

    /* ---------------------------------------------------------------------- */
    /* Step 1. if lo < b < hi, then for each free k,                          */
    /*         see if x_k can be pushed to 0 or 1                             */
    /* ---------------------------------------------------------------------- */

    FreeSet_dump ("QPBoundary start",
        n, LinkUp, LinkDn, nFreeSet, FreeSet_status, 0, x) ;

    printf ("Boundary 1 start: ib %ld lo %g b %g hi %g b-lo %g hi-b %g\n",
        ib, lo, b, hi, b-lo, hi-b) ;

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
                ib = -1;        // TODO ib ok
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
                b = hi;         // TODO ib ok
                x[k] -= s;
            }
        }

        if (ib == 0) /* x_k hits boundary */
        {
            //--- remove k from FreeSet and update its status
            ASSERT(FreeSet_status [k] == 0) ;
            FreeSet_status[k] = new_FreeSet_status_k ;
            ASSERT(FreeSet_status [k] != 0) ;
            nFreeSet--;
            Int h = LinkUp[k];
            Int g = LinkDn[k];
            LinkUp[g] = h;
            LinkDn[h] = g;
            FreeSet_dump ("QPBoundary:0", n, LinkUp, LinkDn,
                nFreeSet, FreeSet_status, 0, x) ;
            //---
            b -= s * ak;        // TODO ib bad?
        }

        for (Int p = Ep[k]; p < Ep[k+1]; p++)
        {
            grad[Ei[p]] += s * Ex[p]; // TODO allow Ex NULL (means all 1's)
        }

        grad[k] += s * D[k];

        printf ("Boundary 1: ib %ld lo %g b %g hi %g b-lo %g hi-b %g\n",
            ib, lo, b, hi, b-lo, hi-b) ;
        QPcheckCom (G, O, QP, 1, nFreeSet, b) ;         // check b
    }

    /* ---------------------------------------------------------------------- */
    /* Step 2. Examine flips of x_k from 0 to 1 or from 1 to 0 */
    /* ---------------------------------------------------------------------- */

    printf ("Boundary step 2:\n") ;

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
            ASSERT (x [k] == 1.) ;
            if (b - ak >= lo)
            {
                if (0.5 * D[k] + grad[k] >= 0) /* flip lowers cost */
                {
                    // TODO dangerous roundoff for b==lo test
                    b -= ak;                    // TODO ib bad?
                    ib = (b == lo ? -1 : 0);
                    x[k] = 0.0 ;
                    FreeSet_status[k] = -1;
                }
            }
        }
        else /* try changing x_k from 0 to 1 */
        {
            ASSERT (x [k] == 0.) ;
            if (b + ak <= hi)
            {
                if (grad[k] - 0.5 * D[k] <= 0) /* flip lowers cost */
                {
                    // TODO dangerous roundoff for b, and b==hi test
                    b += ak;                    // TODO ib bad?
                    ib = (b == hi ? 1 : 0);
                    x[k] = 1.0 ;
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
                    grad[Ei[p]] += Ex[p];   // TODO allow Ex NULL (all 1's)
                }
                grad[k] += D[k];
            }
            else /* x [k] was 0, now it is 1 */
            {
                for (Int p = Ep[k]; p < Ep[k+1]; p++)
                {
                    grad[Ei[p]] -= Ex[p];   // TODO allow Ex NULL (all 1's)
                }
                grad[k] -= D[k];
            }
        }
        QPcheckCom (G, O, QP, 1, nFreeSet, b) ;         // check b
    }

    /* ---------------------------------------------------------------------- */
    // quick return if FreeSet is now empty
    /* ---------------------------------------------------------------------- */

    if (nFreeSet == 0)
    {
        printf ("Boundary quick: ib %ld lo %g b %g hi %g b-lo %g hi-b %g\n",
            ib, lo, b, hi, b-lo, hi-b) ;
        QP->nFreeSet = nFreeSet;
        QP->b = b;
        QP->ib = ib;
        printf ("------- QPBoundary end ]\n") ;
        return;
    }

    /* ---------------------------------------------------------------------- */
    /* Step 3. Search for a_{ij} = 0 in the free index set */
    /* ---------------------------------------------------------------------- */

    // look for where both i and j are in the FreeSet,
    // but i and j are not adjacent in the graph G.

    FreeSet_dump ("step 3", n, LinkUp, LinkDn, nFreeSet, FreeSet_status, 0, x) ;

    // for each j in FreeSet, and while |FreeSet| > 1:
    for (Int j = LinkUp[n]; (j < n) && (nFreeSet > 1) ; j = LinkUp[j])
    {
#ifndef NDEBUG
        // printf ("j %g nFreeSet %g\n", (double) j, (double) nFreeSet) ;
#endif

        // TODO merge this loop with the next one.  Clear marks if continue.
        // count how many neighbors of j are free
        Int m = 1;                                  // (including j itself)
        for (Int p = Ep[j]; p < Ep[j+1]; p++)
        {
            Int i = Ei[p] ;
            ASSERT(i != j) ;                       // graph has no self edges
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
            ASSERT(i != j) ;                       // graph has no self edges
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
                    grad[Ei[p]] -= Ex[p] * dxj; // TODO allow Ex NULL (all 1's)
                }
                for (Int p = Ep[i]; p < Ep[i+1]; p++) 
                {
                    grad[Ei[p]] -= Ex[p] * dxi; // TODO allow Ex NULL (all 1's)
                }
                grad[j] -= D[j] * dxj;
                grad[i] -= D[i] * dxi;

                // Remove either i or j from the FreeSet.  Note that it
                // is possible for both x[i] and x[j] to reach their bounds
                // at the same time.  Only one is removed from the FreeSet;
                // the other will be removed later.

                if (bind1)
                {
                    // remove j from the FreeSet
                    FreeSet_dump ("QPBoundary:1 before", n, LinkUp, LinkDn,
                        nFreeSet, FreeSet_status, 0, NULL) ;
                    ASSERT(FreeSet_status [j] == 0) ;
                    FreeSet_status [j] = new_FreeSet_status ;
                    ASSERT(FreeSet_status [j] != 0) ;
                    nFreeSet--;
                    Int h = LinkUp[j];
                    Int g = LinkDn[j];
                    LinkUp[g] = h;
                    LinkDn[h] = g;
                    FreeSet_dump ("QPBoundary:1", n, LinkUp, LinkDn,
                        nFreeSet, FreeSet_status, 0, x) ;
                    //---
                    break;
                }
                else
                {
                    // remove i from the FreeSet
                    FreeSet_dump ("QPBoundary:2 before", n, LinkUp, LinkDn,
                        nFreeSet, FreeSet_status, 0, NULL) ;
                    ASSERT(FreeSet_status [i] == 0) ;
                    FreeSet_status [i] = new_FreeSet_status ;
                    ASSERT(FreeSet_status [i] != 0) ;
                    nFreeSet--;
                    Int h = LinkUp[i];
                    Int g = LinkDn[i];
                    LinkUp[g] = h;
                    LinkDn[h] = g;
                    FreeSet_dump ("QPBoundary:2", n, LinkUp, LinkDn,
                        nFreeSet, FreeSet_status, 0, x) ;
                    //---
                    continue;
                }
            }
        }

        // clear the marks from all the nodes
        MONGOOSE_CLEAR_ALL_MARKS ;

        QPcheckCom (G, O, QP, 1, nFreeSet, b) ;         // check b
    }

    /* ---------------------------------------------------------------------- */
    // assert that the nodes in the FreeSet form a single clique
    /* ---------------------------------------------------------------------- */

    ASSERT(nFreeSet >= 1) ;    // we can have 1 or more nodes still in FreeSet
#ifndef NDEBUG
    // this test is for debug mode only
    for (Int j = LinkUp[n]; (j < n) ; j = LinkUp[j])
    {
        // j must be adjacent to all other nodes in the FreeSet
        Int nfree_neighbors = 0 ;
        for (Int p = Ep[j]; p < Ep[j+1]; p++)
        {
            Int i = Ei[p] ;
            ASSERT(i != j) ;
            if (FreeSet_status [i] == 0) nfree_neighbors++ ;
        }
        ASSERT(nfree_neighbors == nFreeSet - 1) ;
    }
#endif

    /* ---------------------------------------------------------------------- */
    /* Step 4. dxj = s/aj, dxi = -s/ai, choose s with g_j dxj + g_i dxi <= 0 */
    /* ---------------------------------------------------------------------- */

    FreeSet_dump ("step 4", n, LinkUp, LinkDn, nFreeSet, FreeSet_status, 0, x) ;

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
            grad[Ei[k]] -= Ex[k] * dxj; // TODO allow Ex NULL (all 1's)
        }
        for (Int k = Ep[i]; k < Ep[i+1]; k++)
        {
            grad[Ei[k]] -= Ex[k] * dxi; // TODO allow Ex NULL (all 1's)
        }
        grad[j] -= D[j] * dxj;
        grad[i] -= D[i] * dxi;

        // ---------------------------------------------------------------------
        // the following 2 cases define the next j in the iteration:
        // ---------------------------------------------------------------------

        // Remove either i or j from the FreeSet.  Note that it is possible for
        // both x[i] and x[j] to reach their bounds at the same time.  Only one
        // is removed from the FreeSet; the other will be removed later.

        if (bind1) /* j is bound */
        {
            // remove j from the FreeSet
            FreeSet_dump ("QPBoundary:3 before", n, LinkUp, LinkDn,
                nFreeSet, FreeSet_status, 0, NULL) ;
            ASSERT(FreeSet_status [j] == 0) ;
            FreeSet_status [j] = new_FreeSet_status ;
            ASSERT(FreeSet_status [j] != 0) ;
            nFreeSet--;
            Int h = LinkUp[j];
            Int g = LinkDn[j];
            LinkUp[g] = h;
            LinkDn[h] = g;
            FreeSet_dump ("QPBoundary:3", n, LinkUp, LinkDn,
                nFreeSet, FreeSet_status, 0, x) ;
            //---
            // go to next j in the list
            j = LinkUp [j] ;
        }
        else /* i is bound */
        {
            // remove i from the FreeSet
            FreeSet_dump ("QPBoundary:4 before", n, LinkUp, LinkDn,
                nFreeSet, FreeSet_status, 0, NULL) ;
            ASSERT(FreeSet_status [i] == 0) ;
            FreeSet_status [i] = new_FreeSet_status ;
            ASSERT(FreeSet_status [i] != 0) ;
            nFreeSet--;
            Int h = LinkUp[i];
            Int g = LinkDn[i];
            LinkUp[g] = h;
            LinkDn[h] = g;
            FreeSet_dump ("QPBoundary:4", n, LinkUp, LinkDn,
                nFreeSet, FreeSet_status, 0, x) ;
            //---
            // j is still in the list and remains the same;
            // do not advance to the next in the list
        }

        QPcheckCom (G, O, QP, 1, nFreeSet, b) ;         // check b
    }

    FreeSet_dump ("wrapup", n, LinkUp, LinkDn, nFreeSet, FreeSet_status, 0, x) ;

    /* ---------------------------------------------------------------------- */
    /* step 5: single free variable remaining */
    /* ---------------------------------------------------------------------- */

    ASSERT (nFreeSet == 0 || nFreeSet == 1) ;

    printf ("Step 5: ib %ld lo %g b %g hi %g b-lo %g hi-b %g\n",
            ib, lo, b, hi, b-lo, hi-b) ;

    if (nFreeSet == 1) /* j is free, optimize over x [j] */
    {
        // j is the first and only item in the FreeSet
        Int j = LinkUp [n] ;
#ifndef NDEBUG
        printf ("ONE AND ONLY!! j = %ld x[j] %g\n", j, x [j]) ;
#endif
        Int bind1 = 0;
        Double aj = a[j];
        Double dxj = (hi - b) / aj;
        printf ("dxj %g  x[j] %g  (1-x[j]): %g\n", dxj, x [j], 1-x[j]) ;
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
        printf ("dxi %g  x[j] %g  (-x[j]): %g\n", dxi, x [j], -x[j]) ;
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
                printf ("bind1: xj changes from %g", x[j]) ;
                x[j] += dxj;
                printf (" to %g, b now at hi\n", x[j]) ;
                ib = +1;
                b = hi;             // TODO ib ok
            }
            else
            {
                x[j] = 1.;
                b += dxj * aj;      // TODO ib bad?
                /// remove j from the FreeSet, which is now empty
                printf ("(b5):remove j = %ld from FreeSet, now empty\n", j) ;
                FreeSet_dump ("QPBoundary:5 before", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 0, NULL) ;
                ASSERT(FreeSet_status [j] == 0) ;
                FreeSet_status[j] = 1;
                ASSERT(FreeSet_status [j] != 0) ;
                nFreeSet--;
                LinkUp[n] = n;
                LinkDn[n] = n;
                FreeSet_dump ("QPBoundary:5", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 0, x) ;
                ASSERT(nFreeSet == 0) ;
                //--- 
            }
        }
        else /* x [j] += dxi */
        {
            dxj = dxi;
            if (bind2)
            {
                printf ("bind2: xj changes from %g", x[j]) ;
                x[j] += dxj;
                printf (" to %g, b now at lo\n", x[j]) ;
                ib = -1;
                b = lo;             // TODO ib ok
            }
            else
            {
                x[j] = 0.;
                b += dxj * aj;      // TODO ib bad?
                /// remove j from the FreeSet, which is now empty
                printf ("(b6):remove j = %ld from FreeSet, now empty\n", j) ;
                FreeSet_dump ("QPBoundary:6 before", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 0, NULL) ;
                ASSERT(FreeSet_status [j] == 0) ;
                FreeSet_status[j] = -1;
                ASSERT(FreeSet_status [j] != 0) ;
                nFreeSet--;
                LinkUp[n] = n;
                LinkDn[n] = n;
                FreeSet_dump ("QPBoundary:6", n, LinkUp, LinkDn,
                    nFreeSet, FreeSet_status, 0, x) ;
                ASSERT(nFreeSet == 0) ;
                //---
            }
        }

        if (b <= lo)
        {
            b = lo ;
            ib = -1 ;
        }
        else if (b >= hi)
        {
            b = hi ;
            ib = +1 ;
        }
        else
        {
            ib = 0 ;
        }

        if (dxj != MONGOOSE_ZERO)
        {
            for (Int p = Ep[j]; p < Ep[j+1]; p++)
            {
                grad[Ei[p]] -= Ex[p] * dxj; // TODO allow Ex NULL (all 1's)
            }
            grad[j] -= D[j] * dxj;
        }
    }

    /* ---------------------------------------------------------------------- */
    // wrapup
    /* ---------------------------------------------------------------------- */

    printf ("QBboundary, done:\n") ;
    FreeSet_dump ("QPBoundary: done ", n, LinkUp, LinkDn,
        nFreeSet, FreeSet_status, 0, x) ;

    ASSERT (nFreeSet == 0 || nFreeSet == 1) ;

    printf ("Boundary done: ib %ld lo %g b %g hi %g b-lo %g hi-b %g\n",
            ib, lo, b, hi, b-lo, hi-b) ;

    QP->nFreeSet = nFreeSet;
    QP->b = b;
    QP->ib = ib;

    // clear the marks from all the nodes
    MONGOOSE_CLEAR_ALL_MARKS ;      // TODO: reset if int overflow
    G->markValue = markValue ;

    QPcheckCom (G, O, QP, 1, nFreeSet, b) ;         // check b
    printf ("----- QPboundary end ]\n") ;
}

} // end namespace Mongoose
