#ifndef Mongoose_QPNapsackPair_hpp
#define Mongoose_QPNapsackPair_hpp

#include "Mongoose_Internal.hpp"

namespace Mongoose
{

void QPNapsackPair
(
    Double *x,         /* holds y on input, and the solution x on output */
    Double *lambda,    /* initial guess (input) final value (output) multiplier
                          lambda [0], lambda [1] */
    Int *ix,           /* ix_i = +1, -1, or 0 on input, x_i =1, 0, or 0 < x_i < 1 */
    Int n,             /* # cols ? */
    Int m,             /* # rows ? */
    Double *a,         /* edge weights ? */
    Int *lo,           /* Problem lo limits in 0, 1 */
    Int *hi,           /* Problem hi limits in 0, 1 */
    Double *wx1,       /* Double workspace #1 */
    Int *wi1,          /* Integer workspace #1 */
    Int *wi2           /* Integer workspace #2 */
);

} // end namespace Mongoose

#endif
