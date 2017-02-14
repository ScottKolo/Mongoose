#ifndef Mongoose_QPDelta_hpp
#define Mongoose_QPDelta_hpp

#include "Mongoose_Internal.hpp"

namespace Mongoose
{

class QPDelta
{
public:
    Double *x;             /* current estimate of solution                    */

    // FreeSet:
    Int nFreeSet;          /* number of i such that 0 < x_i < 1               */
    Int *FreeSet_status;   /* ix_i = +1,-1, or 0 if x_i = 1,0, or 0 < x_i < 1 */
    Int *LinkUp;           /* linked list for free indices                    */
    Int *LinkDn;           /* linked list, LinkDn [LinkUp [i]] = i            */
    //---

    Double *gradient;      /* gradient at current x                           */
    Double *D;             /* max value along the column.                     */

    // workspace
    Int *wi[2];
    Double *wx[3];

    Int its;
    Double err;
    Int ib;
    Double b;

    static QPDelta *Create(Int n);
    ~QPDelta();
};

} // end namespace Mongoose

#endif
