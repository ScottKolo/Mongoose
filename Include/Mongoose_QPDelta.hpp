#ifndef Mongoose_QPDelta_hpp
#define Mongoose_QPDelta_hpp

#include "Mongoose_Internal.hpp"

namespace Mongoose
{

class QPDelta
{
public:
    double *x;             /* current estimate of solution                    */

    // FreeSet:
    Int nFreeSet;          /* number of i such that 0 < x_i < 1               */
    Int *FreeSet_status;   /* ix_i = +1,-1, or 0 if x_i = 1,0, or 0 < x_i < 1 */
    Int *FreeSet_list;     /* list for free indices                    */
    //---

    double *gradient;      /* gradient at current x                           */
    double *D;             /* max value along the column.                     */

    double lo ;             // lo <= a'*x <= hi must always hold
    double hi ;

    // workspace
    Int *wi[2];
    double *wx[3];

    Int its;
    double err;
    Int ib;                 // ib =  0 means lo < b < hi
                            // ib = +1 means b == hi
                            // ib = -1 means b == lo
    double b;               // b = a'*x

    double lambda;

    static QPDelta *Create(Int numVars);
    ~QPDelta();

    double check_cost; // For debugging

private:
    static const Int WXSIZE = 2;
    static const Int WISIZE = 3;
};

} // end namespace Mongoose

#endif
