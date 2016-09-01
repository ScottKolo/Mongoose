#ifndef Mongoose_QPNapsack_hpp
#define Mongoose_QPNapsack_hpp

#include "Mongoose_Internal.hpp"

namespace Mongoose
{

Double QPnapsack            /* return the final lambda */
(
    Double *x,              /* holds y on input, and the solution x on output */
    Int n,                  /* size of x, constraint lo <= a'x <= hi */
    Double lo,              /* partition lower bound */
    Double hi,              /* partition upper bound */
    Double *a,              /* vector of nodal weights */
    Double Lambda,          /* initial guess for lambda */
    Int *ix,                /* ix_i = +1,-1, or 0 on input, x_i =1,0, or 0< x_i< 1*/
    Double *w,              /* work array of size n */
    Int *heap1,             /* work array of size n+1 */
    Int *heap2              /* work array of size n+1 */
);

} // end namespace Mongoose

#endif
