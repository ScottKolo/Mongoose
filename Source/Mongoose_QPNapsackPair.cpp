/* ========================================================================== */
/* === QPnapsack2 =========================================================== */
/* ==========================================================================
    Solve a pair of projection problems:

        min ||x_0 - y_0|| subject to 0 <= x_0 <= 1,
                                     a_0'x_0 = b_0, lo_0 <= b_0 <= hi_0

        min ||x_1 - y_1|| subject to 0 <= x_1 <= 1,
                                     a_1'x_1 = b_1, lo_1 <= b_1 <= hi_1

    where x_0 stands for the first m components of an n component vector
    while x_1 stands for the last n-m components.  It is assumed that
    the column vector a is strictly positive since, in our application,
    the vector a represents the node weights, which are >= 1. If a is NULL,
    then it is assumed that a is identically 1.
   ========================================================================== */

#include "Mongoose_QPNapsackPair.hpp"
#include "Mongoose_QPNapsack.hpp"

namespace Mongoose
{

void QPNapsackPair
(
    Double *x,         /* holds y on input, and the solution x on output */
    Double *lambda,    /* initial guess (input) final value (output) multiplier
                          lambda [0], lambda [1] */
    Int *FreeSet_status, /* FreeSet_status = +1, -1, or 0 on input,
                            x_i =1, 0, or 0 < x_i < 1.  Not modified */
    Int n,             /* # cols ? */
    Int m,             /* # rows ? */
    Double *a,         /* edge weights ? */
    Int *lo,           /* Problem lo limits in 0, 1 */
    Int *hi,           /* Problem hi limits in 0, 1 */
    Double *wx1,       /* Double workspace #1 */
    Int *wi1,          /* Integer workspace #1 */
    Int *wi2           /* Integer workspace #2 */
)
{
    Double *a0, *a1, *x0, *x1 ;
    Int *FreeSet_status0, *FreeSet_status1 ;

    if (FreeSet_status != NULL)
    {
        FreeSet_status0 = FreeSet_status ;          // size m
        FreeSet_status1 = FreeSet_status0 + m ;     // size n-m, but only if n-m>0
    }
    else
    {
        FreeSet_status0 = NULL ;                    // not present
        FreeSet_status1 = NULL ;
    }

    if (a != NULL)
    {
        a0 = a ;            // size m
        a1 = a + m ;        // size n-m, but only if n-m > 0
    }
    else
    {
        a0 = NULL ;         // not present
        a1 = NULL ;
    }

    x0 = x ;                // size m
    x1 = x + m ;            // size n-m, but only if n-m > 0

    lambda[0] = QPnapsack(x0, m, lo[0], hi[0], a0, lambda[0],
        FreeSet_status0, wx1, wi1, wi2);

    if (m >= n) return;

    lambda[1] = QPnapsack(x1, n-m, lo[1], hi[1], a1, lambda[1], 
        FreeSet_status1, wx1, wi1, wi2);
}

} // end namespace Mongoose
