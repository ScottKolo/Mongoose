/**
 * @file Mongoose_QPGradProj.cpp
 * @author S. Nuri Yeralan
 * @brief Gradient projection algorithm
 *
 * @details Apply gradient projection algorithm to the quadratic program which
 * arises in graph partitioning:
 *
 * min (1-x)'(D+A)x subject to lo <= b <= hi, a'x = b, 0 <= x <= 1
 *
 * The gradient at the current point is provided as input, and the
 * gradient is updated in each iteration.
 */

#include "Mongoose_QPGradProj.hpp"
#include "Mongoose_QPNapsack.hpp"
#include "Mongoose_QPBoundary.hpp"

namespace Mongoose
{

Double QPgradproj
(
    Graph *G,
    Options *O,
    QPDelta *QP
)
{

    printf ("\n------- QPGradProj start: [\n") ;
    QPcheckCom (G, O, QP, 0, QP->nFreeSet, -999999) ;        // do not check b

    /* ---------------------------------------------------------------------- */
    /* Unpack the relevant structures                                         */
    /* ---------------------------------------------------------------------- */
    Double tol = O->gradprojTol;
    Double *wx1 = QP->wx[0];    /* Double work array, used in napsack and main program     */
    Double *wx2 = QP->wx[1];    /* Double work array, used in napsack and as Dgrad in main */
    Double *wx3 = QP->wx[2];    /* Double work array, used in main for y - x               */
    Int *wi1 = QP->wi[0];       /* Integer work array, used in napsack and as C in main    */
    Int *wi2 = QP->wi[1];       /* Integer work array, used in napsack                     */

    /* Output and Input */
    Double *x = QP->x;             /* current estimate of solution            */
    Int *FreeSet_status = QP->FreeSet_status;
        /* FreeSet_status [i] = +1,-1, or 0 if x_i = 1,0, or 0 < x_i < 1 */

    Int nFreeSet = QP->nFreeSet;    /* number of i such that 0 < x_i < 1 */
    Int *LinkUp = QP->LinkUp;      /* linked list for free indices            */
    Int *LinkDn = QP->LinkDn;      /* linked list, LinkDn [LinkUp [i]] = i    */

    Double *grad = QP->gradient;          /* gradient at current x                   */

    /* Unpack the problem's parameters. */
    Int n = G->n;                  /* problem dimension */
    Int *Ep = G->p;                /* points into Ex or Ei */
    Int *Ei = G->i;                /* adjacent vertices for each node */
    Weight *Ex = G->x;             /* edge weights */
    Weight *Ew = G->w;             /* node weights; a'x = b, lo <= b <= hi */

    Double lo = QP->lo ;
    Double hi = QP->hi ;
//  Double lo = G->W *
//              (O->targetSplit <= 0.5 ? O->targetSplit : 1 - O->targetSplit);
//  Double hi = G->W *
//              (O->targetSplit >= 0.5 ? O->targetSplit : 1 - O->targetSplit);

    Double *D = QP->D; /* diagonal of quadratic */

    /* gradient projection parameters */
    Int limit = O->gradprojIterationLimit; /* max number of iterations */

    /* work arrays */
    Double *y = wx1;
    Double *wx = wx2;
    Double *d = wx3;
    Double *Dgrad = wx;     /* gradient change       ; used in napsack as wx  */
    Int *C = wi1;           /* components of x change; used in napsack as wi1 */

    /* compute error, take step along projected gradient */
    Int ib = 0;             /* initialize ib so that lo < b < hi */
    Double lambda = MONGOOSE_ZERO;
    Int it = 0;
    Double err = INFINITY;

    FreeSet_dump ("QPGradProj: start", n, LinkUp, LinkDn, nFreeSet,
        FreeSet_status, 0, x) ;

    // printf ("\nFunky: n %ld  nFreeSet: %ld\n", n, nFreeSet) ;

#if 0
    // TODO: how does this work?  Doesn't it change the FreeSet ??? 
    // Is the free set guaranteed to be empty at this point ??
    // Or does this not change the FreeSet ??
    for (Int k = 0; k < n; k++)
    {
        fflush (stdout) ; fflush (stderr) ; 
        printf ("x [%ld] =  %g  FreeSet_status %ld\n ", k, x [k], FreeSet_status [k]) ;
        if (x [k] >= MONGOOSE_ONE)
        {
            if (FreeSet_status [k] != +1) printf ("\n%e : CHANGE %ld to +1\n", x [k], FreeSet_status [k]) ;
            fflush (stdout) ; fflush (stderr) ; 
            ASSERT(FreeSet_status [k] == +1 || FreeSet_status [k] == -1) ;
            FreeSet_status [k] = +1 ;
        }
        else if (x [k] <= MONGOOSE_ZERO)
        {
            if (FreeSet_status [k] != -1) printf ("\n%e : CHANGE %ld to -1\n", x [k], FreeSet_status [k]) ;
            fflush (stdout) ; fflush (stderr) ; 
            ASSERT (FreeSet_status [k] == +1 || FreeSet_status [k] == -1) ;
            FreeSet_status [k] = -1 ;
        }
        else
        {
            if (FreeSet_status [k] !=  0)
            {
                printf ("\nCHANGE to 0\n") ;
            }
            fflush (stdout) ; fflush (stderr) ; 
            ASSERT(FreeSet_status [k] == 0) ;  // ???
            FreeSet_status [k] = 0 ;
        }
        printf ("\n") ;
    }
    FreeSet_dump ("QPGradProj:1", n, LinkUp, LinkDn, nFreeSet,
        FreeSet_status, 0, x) ;
#endif

    while (err > tol)
    {

        printf ("top of QPgrad while loop\n") ;
        FreeSet_dump ("QPGradProj:0", n, LinkUp, LinkDn, nFreeSet,
            FreeSet_status, 0, x) ;
        QPcheckCom (G, O, QP, 0, QP->nFreeSet, -999999) ;   // do not check b

        // check grad
        {
            // for debugging, just use malloc
            Double s = 0. ;
            Double *mygrad = (Double *) malloc ((n+1)*sizeof(Double)) ;
            for (Int k = 0; k < n; k++)
            {
                mygrad[k] = (0.5-x[k]) * D[k];
            }
            for (Int k = 0; k < n; k++)
            {
                Double xk = x[k];
                s += Ew[k] * xk;
                Weight r = 0.5 - xk;
                for (Int p = Ep[k]; p < Ep[k+1]; p++)
                {
                    mygrad[Ei[p]] += r * Ex[p];
                }
            }
            Double maxerr = 0. ;
            for (Int k = 0; k < n; k++)
            {
                Double err = fabs (grad [k]-mygrad [k]) ;
                maxerr = MONGOOSE_MAX2 (maxerr, err) ;
            }
            // printf ("check grad %g\n", maxerr) ;
            ASSERT (maxerr < tol) ;
            free (mygrad) ;
        }

        /* Moving in the gradient direction. */
        for (Int k = 0; k < n; k++) y[k] = x[k] - grad[k];

        /* Run the napsack. */
        lambda = QPnapsack(y, n, lo, hi, Ew, lambda,
            FreeSet_status, wx, wi1, wi2);

        // TODO: This can probably be done in the napsack to avoid O(n) loop
        /* Compute the maximum error. */
        err = -INFINITY;
        for (Int k = 0; k < n; k++) err = MONGOOSE_MAX2(err, fabs(y[k]-x[k]));

        /* If we converged or got exhausted, save context and exit. */
        if ((err <= tol) || (it >= limit))
        {
            printf ("QPGradProj exhausted:") ;
            saveContext(G, QP, it, err, nFreeSet, ib, lo, hi);
            QPcheckCom (G, O, QP, 1, QP->nFreeSet, QP->b) ; // check b
            FreeSet_dump ("QPGradProj exhausted", n, LinkUp, LinkDn, nFreeSet,
                FreeSet_status, 0, x) ;
            printf ("------- QPGradProj end ]\n") ;
            return err;
        }

        it++;

        /* compute stepsize st = g_F'g_F/-g_F'(A+D)g_F */
        /* TODO: Can Dgrad be cleared after use to avoid O(n)? */
        for (Int k = 0; k < n; k++) Dgrad[k] = MONGOOSE_ZERO;

        FreeSet_dump ("QPGradProj:1", n, LinkUp, LinkDn, nFreeSet,
            FreeSet_status, 0, x) ;

        // for each i in the FreeSet:
        for (Int i = LinkUp[n]; i < n ; i = LinkUp[i])
        {
            /* compute -(A+D)g_F */
            Double s = grad[i];
            for (Int p = Ep[i]; p < Ep[i+1]; p++)
            {
                Dgrad[Ei[p]] -= s * Ex[p];      // TODO all Ex NULL (all 1s)
            }
            Dgrad[i] -= s * D[i];
        }

        Double st_num = MONGOOSE_ZERO;
        Double st_den = MONGOOSE_ZERO;

        FreeSet_dump ("QPGradProj:2", n, LinkUp, LinkDn, nFreeSet,
            FreeSet_status, 0, x) ;

        for (Int j = LinkUp[n]; j < n ; j = LinkUp[j])
        {
            st_num += grad[j] * grad[j];
            st_den += grad[j] * Dgrad[j];
        }

//      printf ("ST_DEN %g\n", st_den) ;

        /* st = g_F'g_F/-g_F'(A+D)g_F unless the denominator <= 0 */
        if (st_den > MONGOOSE_ZERO)
        {
//          printf ("change y\n") ;
            Double st = MONGOOSE_MAX2 (st_num / st_den, 0.001);
            for (Int j = 0; j < n; j++) y[j] = x[j] - st * grad[j];
            lambda = QPnapsack(y, n, lo, hi, Ew, lambda,
                FreeSet_status, wx, wi1, wi2);
        }

        /* otherwise st = 1 and y is as computed above */
        Int nc = 0; /* number of changes (number of j for which y_j != x_j) */
        Weight s = MONGOOSE_ZERO;
        for (Int j = 0; j < n; j++) Dgrad[j] = MONGOOSE_ZERO;
        for (Int j = 0; j < n; j++)
        {
            Double t = y[j] - x[j];
            if (t != MONGOOSE_ZERO)
            {
                // printf ("j %ld x[j] %g y[j] %g\n", j, x[j], y[j]) ;
                d[j] = t;
                s += t * grad[j]; /* derivative in the direction y - x */
//              printf ("C: we shall consider j %ld t %g\n", j, t) ;
                C[nc] = j;
                nc++;
                for (Int p = Ep[j]; p < Ep[j+1]; p++)
                {
                    Dgrad[Ei[p]] -= Ex[p] * t;      // TODO all Ex NULL (all 1s)
                }
                Dgrad[j] -= D[j] * t;
            }
        }

        // printf ("directional derivative s = %g\n", s) ;

        /* If directional derivative has wrong sign, save context and exit. */
        if (s >= MONGOOSE_ZERO)
        {
            printf ("QPGradProj directional derivative has wrong sign\n") ;
            saveContext(G, QP, it, err, nFreeSet, ib, lo, hi);
            FreeSet_dump ("QPGradProj wrong sign", n, LinkUp, LinkDn, nFreeSet,
                FreeSet_status, 0, x) ;
            printf ("------- QPGradProj end ]\n") ;
            return err;
        }

        // HERE, lo <= a'y <= hi should hold
        {
            Double aty = 0., atx = 0. ;
            for (Int j = 0 ; j < n ; j++)
            {
                aty += Ew [j] * y [j] ;
                atx += Ew [j] * x [j] ;
            }
            int good_aty = (lo-tol <= aty) && (aty <= hi+tol) ;
            int good_atx = (lo-tol <= atx) && (atx <= hi+tol) ;
            if (!good_aty || !good_atx)
            {
                if (!good_aty)
                {
                    printf ("BAD ATY: lo %g a'y %g hi %g tol %g\n", lo, aty, hi, tol) ;
                }
                if (!good_atx)
                {
                    printf ("BAD ATX: lo %g a'y %g hi %g tol %g\n", lo, atx, hi, tol) ;
                }
                fflush (stdout) ;
                fflush (stderr) ;
            }
            ASSERT (lo-tol <= aty) ;
            ASSERT (aty <= hi+tol) ;
            ASSERT (lo-tol <= atx) ;
            ASSERT (atx <= hi+tol) ;
        }

        Double t = MONGOOSE_ZERO;
        for (Int k = 0; k < nc; k++)
        {
            Int j = C[k];
            t += Dgrad[j] * d[j]; /* -dg'd */
        }

        // printf ("MIN ATTAINED AT Y? s %g t %g s+t %g\n", s, t, s+t) ;

        if (s+t <= 0) /* min attained at y, slope at y <= 0 */
        {
            // printf ("min attained at y: s %g t %g s+t %g\n", s, t, s+t) ;
            ib = (lambda > 0 ? 1 : lambda < 0 ? -1 : 0);
            for (Int k = 0; k < nc; k++)
            {
                Int j = C[k];
                Double yj = y[j];
                x[j] = yj;

                Int bind ; /* -1 = no change, 0 = free, +1 = bind */

                Int FreeSet_status_j = FreeSet_status[j] ;
                if (FreeSet_status_j > 0)
                {
                    if (yj == MONGOOSE_ZERO)
                    {
                        // status of j changes from +1 to -1.  no change to FreeSet
                        FreeSet_status_j = -1;
                        bind = -1 ;
                    }
                    else
                    {
                        // status of j changes from +1 to 0.  add j to FreeSet
                        FreeSet_status_j = 0;
                        bind = 0 ;
                    }
                }
                else if (FreeSet_status_j < 0)
                {
                    if (yj == 1.0)
                    {
                        // status of j changes from -1 to 1.  no change to FreeSet
                        FreeSet_status_j = 1;
                        bind = -1 ;
                    }
                    else
                    {
                        // status of j changes from -1 to 0.  add j to FreeSet
                        FreeSet_status_j = 0;
                        bind = 0;
                    }
                }
                else /* x_j currently free, but it may become bound */
                {
                    if (yj == 1.0) /* x_j hits upper bound */
                    {
                        // status of j changes from 0 to 1,  remove from FreeSet
                        FreeSet_status_j = 1;
                        bind = 1;
                    }
                    else if (yj == MONGOOSE_ZERO) /* x_j hits lower bound */
                    {
                        // status of j changes from 0 to -1,  remove from FreeSet
                        FreeSet_status_j = -1;
                        bind = 1;
                    }
                    else
                    {
                        // status of j remains 0.  no change to FreeSet
                        FreeSet_status_j = 0 ;
                        bind = -1 ;
                    }
                }

                if (bind == 0)
                {
                    // add j to the FreeSet
                    FreeSet_dump ("QPGradProj:3 before", n, LinkUp,
                        LinkDn, nFreeSet, FreeSet_status, 0, NULL) ;
                    ASSERT (FreeSet_status [j] != 0) ;
                    FreeSet_status[j] = 0;
                    nFreeSet++;
                    Int m = LinkUp[n];
                    LinkUp[j] = m;
                    LinkUp[n] = j;
                    LinkDn[m] = j;
                    LinkDn[j] = n;
                    FreeSet_dump ("QPGradProj:3 after", n,
                        LinkUp, LinkDn, nFreeSet, FreeSet_status, 0, x) ;
                    //---
                }
                else if (bind == 1)
                {
                    // remove j from the FreeSet
                    FreeSet_dump ("QPGradProj:4 before", n,
                        LinkUp, LinkDn, nFreeSet, FreeSet_status, 0, NULL) ;
                    ASSERT (FreeSet_status [j] == 0) ;
                    FreeSet_status [j] = FreeSet_status_j ;
                    ASSERT (FreeSet_status [j] != 0) ;
                    nFreeSet--;
                    Int h = LinkUp[j];
                    Int g = LinkDn[j];
                    LinkUp[g] = h;
                    LinkDn[h] = g;
                    FreeSet_dump ("QPGradProj:4", n,
                        LinkUp, LinkDn, nFreeSet, FreeSet_status, 0, x) ;
                    //---
                }
                else // bind == -1, no change to the FreeSet
                {
                    FreeSet_status [j] = FreeSet_status_j ;
                }
            }
            for (Int j = 0; j < n; j++)
            {
                grad[j] += Dgrad[j];
            }
        }
        else /* partial step towards y, st < 1 */
        {
            if ((ib > 0 && lambda <= 0) || (ib < 0 && lambda >= 0))
            {
                ib = 0;
            }

            Double st = -s / t;
            // printf ("partial step towards y, st %g\n", st) ;
            for (Int k = 0; k < nc; k++)
            {
                Int j = C[k];
                if (FreeSet_status[j] != 0) /* x_j became free */
                {
                    // add j to the FreeSet
                    FreeSet_dump ("QPGradProj:5 before", n, LinkUp, LinkDn,
                        nFreeSet, FreeSet_status, 0, NULL) ;
                    ASSERT (FreeSet_status [j] != 0) ;
                    FreeSet_status[j] = 0;
                    nFreeSet++;
                    Int m = LinkUp[n];
                    LinkUp[j] = m;
                    LinkUp[n] = j;
                    LinkDn[m] = j;
                    LinkDn[j] = n;
                    FreeSet_dump ("QPGradProj:5", n, LinkUp, LinkDn,
                        nFreeSet, FreeSet_status, 0, x) ;
                    //---
                }

                /*  else x_j is free before and after step */
                x[j] += st * d[j];
            }

            for (Int k = 0; k < n; k++)
            {
                grad[k] += st * Dgrad[k];
            }
        }

        FreeSet_dump ("QPGradProj:6", n, LinkUp, LinkDn, nFreeSet,
            FreeSet_status, 0, x) ;

        // do not check b
        printf ("QPGradProj continues:\n") ;
        QP->nFreeSet = nFreeSet ;
        QPcheckCom (G, O, QP, 0, QP->nFreeSet, -999999) ;   // do not check b
    }

    FreeSet_dump ("QPGradProj end", n, LinkUp, LinkDn, nFreeSet,
        FreeSet_status, 0, x) ;

    printf ("------- QPGradProj end ]\n") ;
    return err;
}

} // end namespace Mongoose
