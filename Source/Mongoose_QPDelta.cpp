
#include "Mongoose_Internal.hpp"
#include "Mongoose_QPDelta.hpp"

namespace Mongoose
{

QPDelta* QPDelta::Create(Int numVars)
{
    QPDelta *ret = (QPDelta*) SuiteSparse_calloc(1, sizeof(QPDelta));
    if(!ret) return NULL;

    ret->x = (double*) SuiteSparse_malloc(numVars, sizeof(double));
    ret->FreeSet_status = (Int*) SuiteSparse_malloc(numVars, sizeof(Int));
    ret->FreeSet_list = (Int*) SuiteSparse_malloc(numVars+1, sizeof(Int));
    ret->gradient = (double*) SuiteSparse_malloc(numVars, sizeof(double));
    ret->D = (double*) SuiteSparse_malloc(numVars, sizeof(double));

    for (int i = 0; i < WISIZE; i++)
    {
        ret->wi[i] = (Int*) SuiteSparse_malloc(numVars+1, sizeof(Int));
    }

    for (Int i = 0; i < WXSIZE; i++)
    {
        ret->wx[i] = (double*) SuiteSparse_malloc(numVars, sizeof(double));
    }

#ifndef NDEBUG
    ret->check_cost = INFINITY;
#endif

    if(!ret->x || !ret->FreeSet_status || !ret->FreeSet_list
    || !ret->gradient || !ret->D || !ret->wi[0] || !ret->wi[1]
    //|| !ret->Change_location
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
    //Change_location = (Int*) SuiteSparse_free(Change_location);

    for(Int i = 0; i < WISIZE; i++)
    {
        wi[i] = (Int*) SuiteSparse_free(wi[i]);
    }

    for(Int i = 0; i < WXSIZE; i++)
    {
        wx[i] = (double *) SuiteSparse_free(wx[i]);
    }
}

} // end namespace Mongoose
