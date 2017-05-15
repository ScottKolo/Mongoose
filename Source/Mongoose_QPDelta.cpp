
#include "Mongoose_Internal.hpp"
#include "Mongoose_QPDelta.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

QPDelta* QPDelta::Create(Int n)
{
    QPDelta *ret = (QPDelta*) SuiteSparse_calloc(1, sizeof(QPDelta));
    if(!ret) return NULL;

    ret->x = (double*) SuiteSparse_malloc(n, sizeof(double));
    ret->FreeSet_status = (Int*) SuiteSparse_malloc(n, sizeof(Int));
    ret->FreeSet_list = (Int*) SuiteSparse_malloc(n+1, sizeof(Int));
    ret->gradient = (double*) SuiteSparse_malloc(n, sizeof(double));
    ret->D = (double*) SuiteSparse_malloc(n, sizeof(double));

    // TODO use WISIZE and WXSIZE here:
    ret->wi[0] = (Int*) SuiteSparse_malloc(n+1, sizeof(Int));
    ret->wi[1] = (Int*) SuiteSparse_malloc(n+1, sizeof(Int));
    ret->wx[0] = (double*) SuiteSparse_malloc(n, sizeof(double));
    ret->wx[1] = (double*) SuiteSparse_malloc(n, sizeof(double));
    ret->wx[2] = (double*) SuiteSparse_malloc(n, sizeof(double));

    // TODO: can this use wi[1] instead?  I think so.
    ret->Change_location = (Int*) SuiteSparse_malloc(n+1, sizeof(Int));

    ret->check_cost = INFINITY ;

    if(!ret->x || !ret->FreeSet_status || !ret->FreeSet_list
    || !ret->gradient || !ret->D || !ret->wi[0] || !ret->wi[1]
    || !ret->Change_location
    || !ret->wx[0] || !ret->wx[1] || !ret->wx[2])        
    {
        ret->~QPDelta();
        ret = (QPDelta*) SuiteSparse_free(ret);
    }

    return ret;   
}

QPDelta::~QPDelta()
{
    x = (double*) SuiteSparse_free(x);
    FreeSet_status = (Int*) SuiteSparse_free(FreeSet_status);
    FreeSet_list = (Int*) SuiteSparse_free(FreeSet_list);
    gradient = (double*) SuiteSparse_free(gradient);
    D = (double*) SuiteSparse_free(D);
    Change_location = (Int*) SuiteSparse_free(Change_location);

    // TODO use WISIZE and WXSIZE here:
    for(Int i=0; i<2; i++)
        wi[i] = (Int*) SuiteSparse_free(wi[i]);

    for(Int i=0; i<3; i++)
        wx[i] = (double*) SuiteSparse_free(wx[i]);
}

} // end namespace Mongoose
