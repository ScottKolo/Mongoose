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
    Int *FreeSet_list;     /* list for free indices                    */
    //---

    Double *gradient;      /* gradient at current x                           */
    Double *D;             /* max value along the column.                     */

    Double lo ;             // lo <= a'*x <= hi must always hold
    Double hi ;

    // workspace
    Int *Change_location ;
    Int *wi[2];
    Double *wx[3];

    Int its;
    Double err;
    Int ib;                 // ib =  0 means lo < b < hi
                            // ib = +1 means b == hi
                            // ib = -1 means b == lo
    Double b;               // b = a'*x

    Double check_cost ;     // for debugging only
    Double lambda;

    static QPDelta *Create(Int n);
    ~QPDelta();
};

} // end namespace Mongoose

#endif
