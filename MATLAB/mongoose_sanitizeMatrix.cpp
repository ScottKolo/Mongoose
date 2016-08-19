
#include "mongoose_mex.hpp"
#include "mongoose_sanitize.hpp"

using namespace Mongoose;

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    const char* usage = "Usage: A_safe = mongoose_sanitizeMatrix(A)";
    if(nargout != 1) mexErrMsgTxt(usage);
    const mxArray *A_matlab = pargin[0];
    cs *A = (cs *) SuiteSparse_malloc(1, sizeof(cs));
    A = cs_mex_get_sparse (A, 0, 1, A_matlab);

    cs *A_safe = sanitize_matrix(A, false);
    A->p = NULL;
    A->i = NULL;
    A->x = NULL;
    cs_spfree(A);

    pargout [0] = cs_mex_put_sparse (&A_safe) ;
}
