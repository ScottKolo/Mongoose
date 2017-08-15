/* ========================================================================== */
/* === QPNapDown ============================================================ */
/* ========================================================================== */

/* Find x that minimizes ||x-y|| while satisfying the constraints
   0 <= x <= 1, a'x = b.
   The algorithm is described in the napsack comments.
   It is assumed that the starting guess lambda for the dual multiplier is >=
   the correct multiplier. Hence, lambda will be decreased.  The slope of the
   dual function, neglecting b, starts out smaller than b. We stop
   when we reach b. We assume that a >= 0, so that as lambda decreases,
   x_i (lambda) increases. Hence, the only bound variables that can become
   free are those with x_i (lambda) <= 0 */

#include "Mongoose_Internal.hpp"
#include "Mongoose_QPNapDown.hpp"
#include "Mongoose_QPMaxHeap.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

double QPNapDown            /* return lambda */
        (
                double *x,              /* holds y on input, not modified */
                Int n,                  /* size of x */
                double lambda,          /* initial guess for the shift */
                double *a,              /* input constraint vector */
                double b,               /* input constraint scalar */
                double *breakpts,       /* break points */
                Int *bound_heap,        /* work array */
                Int *free_heap          /* work array */
        )
{
    ASSERT(a != NULL);
    Int i, k, e, maxsteps, n_bound, n_free;
    double ai, asum, a2sum, maxbound, maxfree, t;

    maxbound = -INFINITY;
    maxfree = -INFINITY;

    /* -------------------------------------------------------------- */
    /* construct the heaps */
    /* -------------------------------------------------------------- */

    n_bound = 0;
    n_free = 0;
    asum = 0.;
    a2sum = 0.;
    for (i = 0; i < n; i++)
    {
        ai = a[i];
        double xi = x[i] - ai * lambda;
        if (xi < 0.)
        {
            n_bound++;
            bound_heap[n_bound] = i;
            t = x[i] / ai;
            maxbound = std::max(maxbound, t);
            breakpts[i] = t;
        }
        else if (xi < 1.)
        {
            n_free++;
            free_heap[n_free] = i;
            t = (x[i] - 1.) / ai;
            asum += x[i] * ai;
            a2sum += ai * ai;
            maxfree = std::max(maxfree, t);
            breakpts[i] = t;
        }
        else
            asum += ai;
    }

    /*------------------------------------------------------------------- */
    /* check to see if zero slope achieved without changing the free set  */
    /* remember that the slope must always be adjusted by b               */
    /*------------------------------------------------------------------- */

    maxsteps = 2 * n + 1;
    for (k = 1; k <= maxsteps; k++)
    {
        double new_break = std::max(maxfree, maxbound);
        double s = asum - new_break * a2sum;
        if ((s >= b) || (new_break == -INFINITY)) /* done */
        {
            if (a2sum != 0.) lambda = (asum - b) / a2sum;
            return lambda;
        }
        lambda = new_break;

        if (k == 1)
        {
            QPMaxHeap_build(free_heap, n_free, breakpts);
            QPMaxHeap_build(bound_heap, n_bound, breakpts);
        }

        /* -------------------------------------------------------------- */
        /* update the heaps */
        /* -------------------------------------------------------------- */

        if (n_free > 0)
        {
            while (breakpts[e = free_heap[1]] >= lambda)
            {
                ai = a[e];
                a2sum -= ai * ai;
                asum = asum + ai * (1. - x[e]);
                n_free = QPMaxHeap_delete(free_heap, n_free, breakpts);
                if (n_free == 0)
                {
                    a2sum = 0.;
                    break;
                }
            }
        }

        if (n_bound > 0)
        {
            while (breakpts[e = bound_heap[1]] >= lambda)
            {
                n_bound = QPMaxHeap_delete(bound_heap, n_bound, breakpts);
                ai = a[e];
                a2sum += ai * ai;
                asum += ai * x[e];
                t = (x[e] - 1.) / ai;
                breakpts[e] = t;
                n_free = QPMaxHeap_add(e, free_heap, breakpts, n_free);
                if (n_bound == 0)
                    break;
            }
        }

        /*------------------------------------------------------------------- */
        /* get the biggest entry in each heap */
        /*------------------------------------------------------------------- */

        maxfree = (n_free > 0 ? breakpts[free_heap[1]] : -INFINITY);
        maxbound = (n_bound > 0 ? breakpts[bound_heap[1]] : -INFINITY);
    }

    /* This should never happen */
    ASSERT (false) ;
    lambda = 0.;
    return lambda;
}

} // end namespace Mongoose
