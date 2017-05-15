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

#include "Mongoose_Internal.hpp"
#include "Mongoose_QPGradProj.hpp"
#include "Mongoose_QPNapsack.hpp"
#include "Mongoose_QPBoundary.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

#define EMPTY (-1)

namespace Mongoose
{

// save the current state of the solution, just before returning from QPGradProj
inline void saveContext
(
    Graph *G,
    QPDelta *QP, 
    Int it,
    double err,
    Int nFreeSet,
    Int ib,
    double lo,
    double hi
)
{
    QP->its = it;
    QP->err = err;
    QP->nFreeSet = nFreeSet;
    double b = 0.0;
    if(ib != 0)
    {
        b = (ib > 0 ? hi : lo);
    }
    else
    {
        for (Int k = 0; k < G->n; k++) b += G->w[k] * QP->x[k];
    }
    QP->ib = ib;
    QP->b = b;
}

double QPgradproj
(
    Graph *G,
    Options *O,
    QPDelta *QP
)
{

    PR (("\n------- QPGradProj start: [\n")) ;
    DEBUG (QPcheckCom (G, O, QP, 0, QP->nFreeSet, -999999)) ; // do not check b

    /* ---------------------------------------------------------------------- */
    /* Unpack the relevant structures                                         */
    /* ---------------------------------------------------------------------- */

    double tol = O->gradprojTol;
    double *wx1 = QP->wx[0];    /* work array for napsack and here as y */
    double *wx2 = QP->wx[1];    /* work array for napsack and here as Dgrad */
    double *wx3 = QP->wx[2];    /* work array used here for d=y-x */
    Int *wi1 = QP->wi[0];       /* work array for napsack
                                   and here as Change_list */
    Int *wi2 = QP->wi[1];       /* work array only for napsack */

    /* Output and Input */
    double *x = QP->x;             /* current estimate of solution            */
    Int *FreeSet_status = QP->FreeSet_status;
        /* FreeSet_status [i] = +1,-1, or 0 if x_i = 1,0, or 0 < x_i < 1 */

    Int nFreeSet = QP->nFreeSet;    /* number of i such that 0 < x_i < 1 */
    Int *FreeSet_list = QP->FreeSet_list;     /* list of free indices */

    double *grad = QP->gradient;          /* gradient at current x */

    /* Unpack the problem's parameters. */
    Int n = G->n;                  /* problem dimension */
    Int *Ep = G->p;                /* points into Ex or Ei */
    Int *Ei = G->i;                /* adjacent vertices for each node */
    Weight *Ex = G->x;             /* edge weights */
    Weight *Ew = G->w;             /* node weights; a'x = b, lo <= b <= hi */

    double lo = QP->lo ;
    double hi = QP->hi ;

    double *D = QP->D; /* diagonal of quadratic */

    /* gradient projection parameters */
    Int limit = O->gradprojIterationLimit; /* max number of iterations */

    /* work arrays */
    double *y = wx1;
    double *wx = wx2;
    double *d = wx3;
    double *Dgrad = wx;     /* gradient change       ; used in napsack as wx  */

    /* components of x change; used in napsack as wi1 */
    Int *Change_list = wi1;

    // TODO can Change_location use wi2?  I think so.
    Int *Change_location = QP->Change_location ;

    /* compute error, take step along projected gradient */
    Int ib = 0;             /* initialize ib so that lo < b < hi */
    //double lambda = 0.;
    double lambda = QP->lambda;
    Int it = 0;
    double err = INFINITY;

    DEBUG (FreeSet_dump ("QPGradProj: start",
        n, FreeSet_list, nFreeSet, FreeSet_status, 0, x)) ;

    while (err > tol)
    {

        PR (("top of QPgrad while loop\n")) ;
        DEBUG (FreeSet_dump ("QPGradProj:0",
            n, FreeSet_list, nFreeSet, FreeSet_status, 0, x)) ;
        DEBUG (QPcheckCom (G, O, QP, 0, QP->nFreeSet, -999999)) ;

        // check grad
        {
            // for debugging, just use malloc
            double s = 0. ;
            double *mygrad = (double *) malloc ((n+1)*sizeof(double)) ;
            for (Int k = 0; k < n; k++)
            {
                mygrad[k] = (0.5-x[k]) * D[k];
            }
            for (Int k = 0; k < n; k++)
            {
                double xk = x[k];
                s += Ew[k] * xk;
                Weight r = 0.5 - xk;
                for (Int p = Ep[k]; p < Ep[k+1]; p++)
                {
                    mygrad[Ei[p]] += r * Ex[p];
                }
            }
            double maxerr = 0. ;
            for (Int k = 0; k < n; k++)
            {
                double err = fabs (grad [k]-mygrad [k]) ;
                maxerr = MONGOOSE_MAX2 (maxerr, err) ;
            }
            // PR (("check grad %g\n", maxerr)) ;
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
            PR (("QPGradProj exhausted:")) ;
            saveContext(G, QP, it, err, nFreeSet, ib, lo, hi);
            DEBUG (QPcheckCom (G, O, QP, 1, QP->nFreeSet, QP->b)) ;
            DEBUG (FreeSet_dump ("QPGradProj exhausted",
                n, FreeSet_list, nFreeSet, FreeSet_status, 0, x)) ;
            PR (("------- QPGradProj end ]\n")) ;
            return err;
        }

        it++;

        /* compute stepsize st = g_F'g_F/-g_F'(A+D)g_F */
        /* TODO: Can Dgrad be cleared after use to avoid O(n)? */
        for (Int k = 0; k < n; k++) Dgrad[k] = 0.;

        DEBUG (FreeSet_dump ("QPGradProj:1",
            n, FreeSet_list, nFreeSet, FreeSet_status, 0, x)) ;

        // for each i in the FreeSet:
        for (Int ifree = 0 ; ifree < nFreeSet ; ifree++)
        {
            /* compute -(A+D)g_F */
            Int i = FreeSet_list [ifree] ;
            double s = grad[i];
            for (Int p = Ep[i]; p < Ep[i+1]; p++)
            {
                Dgrad[Ei[p]] -= s * Ex[p];      // TODO all Ex NULL (all 1s)
            }
            Dgrad[i] -= s * D[i];
        }

        double st_num = 0.;
        double st_den = 0.;

        DEBUG (FreeSet_dump ("QPGradProj:2",
            n, FreeSet_list, nFreeSet, FreeSet_status, 0, x)) ;

        for (Int jfree = 0 ; jfree < nFreeSet ; jfree++)
        {
            Int j = FreeSet_list [jfree] ;
            st_num += grad[j] * grad[j];
            st_den += grad[j] * Dgrad[j];
        }

        /* st = g_F'g_F/-g_F'(A+D)g_F unless the denominator <= 0 */
        if (st_den > 0.)
        {
            // PR (("change y\n")) ;
            double st = MONGOOSE_MAX2 (st_num / st_den, 0.001);
            for (Int j = 0; j < n; j++) y[j] = x[j] - st * grad[j];
            lambda = QPnapsack(y, n, lo, hi, Ew, lambda,
                FreeSet_status, wx, wi1, wi2);
        }

        /* otherwise st = 1 and y is as computed above */
        Int nc = 0; /* number of changes (number of j for which y_j != x_j) */
        Weight s = 0.;
        for (Int j = 0; j < n; j++) Dgrad[j] = 0.;

        // consider nodes j in the FreeSet_list
        for (Int jfree = 0 ; jfree < nFreeSet ; jfree++)
        {
            Int j = FreeSet_list [jfree] ;
            ASSERT (FreeSet_status [j] == 0) ;
            double t = y[j] - x[j];
            if (t != 0.)
            {
                // PR (("Change_list: we shall consider j %ld t %g\n", j, t)) ;
                d[j] = t;
                s += t * grad[j]; /* derivative in the direction y - x */
                // add j to the Change_list and keep track of its position
                // in the FreeSet_list, in case we need to remove it from
                // the FreeSet.
                Change_list[nc] = j;
                Change_location[j] = jfree ;
                nc++;
                for (Int p = Ep[j]; p < Ep[j+1]; p++)
                {
                    Dgrad[Ei[p]] -= Ex[p] * t;      // TODO all Ex NULL (all 1s)
                }
                Dgrad[j] -= D[j] * t;
            }
        }

        // now consider nodes j not in the FreeSet_list
        for (Int j = 0 ; j < n ; j++)
        {
            if (FreeSet_status [j] == 0)
            {
                // j is in the FreeSet, so skip it (already done above)
                continue ;
            }
            double t = y[j] - x[j];
            if (t != 0.)
            {
                // PR (("Change_list: we shall consider j %ld t %g\n", j, t)) ;
                d[j] = t;
                s += t * grad[j]; /* derivative in the direction y - x */
                Change_list[nc] = j;
                Change_location[j] = EMPTY ;        // j not in FreeSet
                nc++;
                for (Int p = Ep[j]; p < Ep[j+1]; p++)
                {
                    Dgrad[Ei[p]] -= Ex[p] * t;      // TODO all Ex NULL (all 1s)
                }
                Dgrad[j] -= D[j] * t;
            }
        }

        // PR (("directional derivative s = %g\n", s)) ;

        /* If directional derivative has wrong sign, save context and exit. */
        if (s >= 0.)
        {
            PR (("QPGradProj directional derivative has wrong sign\n")) ;
            saveContext(G, QP, it, err, nFreeSet, ib, lo, hi);
            DEBUG (FreeSet_dump ("QPGradProj wrong sign",
                n, FreeSet_list, nFreeSet, FreeSet_status, 0, x)) ;
            PR (("------- QPGradProj end ]\n")) ;
            return err;
        }

#ifndef NDEBUG
        // lo <= a'y <= hi should hold
        {
            double aty = 0., atx = 0. ;
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
                    PR (("BAD ATY: lo %g a'y %g hi %g tol %g\n",
                        lo, aty, hi, tol)) ;
                }
                if (!good_atx)
                {
                    PR (("BAD ATX: lo %g a'y %g hi %g tol %g\n",
                        lo, atx, hi, tol)) ;
                }
                FFLUSH ;
            }
            ASSERT (lo-tol <= aty) ;
            ASSERT (aty <= hi+tol) ;
            ASSERT (lo-tol <= atx) ;
            ASSERT (atx <= hi+tol) ;
        }
#endif

        double t = 0.;
        for (Int k = 0; k < nc; k++)
        {
            Int j = Change_list[k];
            t += Dgrad[j] * d[j]; /* -dg'd */
        }

        // PR (("MIN ATTAINED AT Y? s %g t %g s+t %g\n", s, t, s+t)) ;

        if (s+t <= 0) /* min attained at y, slope at y <= 0 */
        {
            // PR (("min attained at y: s %g t %g s+t %g\n", s, t, s+t)) ;
            ib = (lambda > 0 ? 1 : lambda < 0 ? -1 : 0);
            for (Int k = 0; k < nc; k++)
            {
                Int j = Change_list[k];
                double yj = y[j];
                x[j] = yj;

                Int bind ; /* -1 = no change, 0 = free, +1 = bind */

                Int FreeSet_status_j = FreeSet_status[j] ;
                if (FreeSet_status_j > 0)
                {
                    if (yj == 0.)
                    {
                        // j changes from +1 to -1.  no change to FreeSet
                        FreeSet_status_j = -1;
                        bind = -1 ;
                    }
                    else
                    {
                        // j changes from +1 to 0.  add j to FreeSet
                        FreeSet_status_j = 0;
                        bind = 0 ;
                    }
                }
                else if (FreeSet_status_j < 0)
                {
                    if (yj == 1.0)
                    {
                        // j changes from -1 to 1.  no change to FreeSet
                        FreeSet_status_j = 1;
                        bind = -1 ;
                    }
                    else
                    {
                        // j changes from -1 to 0.  add j to FreeSet
                        FreeSet_status_j = 0;
                        bind = 0;
                    }
                }
                else /* x_j currently free, but it may become bound */
                {
                    if (yj == 1.0) /* x_j hits upper bound */
                    {
                        // j changes from 0 to 1,  remove from FreeSet
                        FreeSet_status_j = 1;
                        bind = 1;
                    }
                    else if (yj == 0.) /* x_j hits lower bound */
                    {
                        // j changes from 0 to -1,  remove from FreeSet
                        FreeSet_status_j = -1;
                        bind = 1;
                    }
                    else
                    {
                        // j remains 0.  no change to FreeSet
                        FreeSet_status_j = 0 ;
                        bind = -1 ;
                    }
                }

                if (bind == 0)
                {
                    // add j to the FreeSet
                    ASSERT (FreeSet_status [j] != 0) ;
                    ASSERT (Change_location [j] == EMPTY) ;
                    FreeSet_status[j] = 0;
                    FreeSet_list [nFreeSet++] = j ;
                    //---
                }
                else if (bind == 1)
                {
                    // remove j from the FreeSet
                    ASSERT (FreeSet_status [j] == 0) ;
                    FreeSet_status [j] = FreeSet_status_j ;
                    ASSERT (FreeSet_status [j] != 0) ;
                    Int jfree = Change_location [j] ;
                    ASSERT (0 <= jfree && jfree < nFreeSet) ;
                    ASSERT (FreeSet_list [jfree] == j) ;
                    FreeSet_list [jfree] = EMPTY ;
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

            double st = -s / t;
            // PR (("partial step towards y, st %g\n", st)) ;
            for (Int k = 0; k < nc; k++)
            {
                Int j = Change_list[k];
                if (FreeSet_status[j] != 0) /* x_j became free */
                {
                    // add j to the FreeSet
                    ASSERT (FreeSet_status [j] != 0) ;
                    ASSERT (Change_location [j] == EMPTY) ;
                    FreeSet_status[j] = 0;
                    FreeSet_list [nFreeSet++] = j ;
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

        // prune any EMPTY entries from the FreeSet
        Int jfree2 = 0 ;
        for (Int jfree = 0 ; jfree < nFreeSet ; jfree++)
        {
            Int j = FreeSet_list [jfree] ;
            if (j != EMPTY)
            {
                ASSERT (FreeSet_status [j] == 0) ;
                FreeSet_list [jfree2++] = j ;
            }
        }
        nFreeSet = jfree2 ;

        DEBUG (FreeSet_dump ("QPGradProj:6",
            n, FreeSet_list, nFreeSet, FreeSet_status, 0, x)) ;

        // do not check b
        PR (("QPGradProj continues:\n")) ;
        QP->nFreeSet = nFreeSet ;
        DEBUG (QPcheckCom (G, O, QP, 0, QP->nFreeSet, -999999)) ;
    }

    DEBUG (FreeSet_dump ("QPGradProj end",
        n, FreeSet_list, nFreeSet, FreeSet_status, 0, x)) ;

    PR (("------- QPGradProj end ]\n")) ;
    return err;
}

} // end namespace Mongoose
