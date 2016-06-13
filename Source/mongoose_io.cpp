
#include "mongoose_io.hpp"
#include "mongoose_interop.hpp"
#include <cstdio>
#include <iostream>
extern "C" {
#include "mmio.h"
}

using namespace std;

namespace Mongoose
{

Graph *read_graph (const char* filename)
{
    cs* A = read_matrix(filename);
    Graph *G = CSparse3ToGraph(A);
    return G;
}

cs *read_matrix (const char* filename)
{
    FILE *file = fopen(filename,"r");
    if (file == NULL)
    {
        cout << "Error: Cannot find file " << filename << endl;
        return NULL;
    }

    MM_typecode matcode;
    if (mm_read_banner(file, &matcode) != 0)
    {
        cout << "Could not process Matrix Market banner." << endl;
        return NULL;
    }
    if (!mm_is_matrix(matcode) || !mm_is_sparse(matcode) || 
        mm_is_complex(matcode))
    {
        cout << "Unsupported matrix format - Must be real and sparse." << endl;
    }

    Int M, N, nz;
    int ret_code;
    if ((ret_code = mm_read_mtx_crd_size(file, &M, &N, &nz)) !=0)
    {
        cout << "Could not parse matrix dimension and size." << endl;
        return NULL;
    }
    
    Int *I = (Int *) SuiteSparse_malloc(nz, sizeof(Int));
    Int *J = (Int *) SuiteSparse_malloc(nz, sizeof(Int));
    Weight *val = (Weight *) SuiteSparse_malloc(nz, sizeof(Weight));

    mm_read_mtx_crd_data(file, M, N, nz, I, J, val, matcode);
    for (Int k = 0; k < nz; k++)
    {
        --I[k];
        --J[k];
    }

    cs *A = (cs *) SuiteSparse_malloc(1, sizeof(cs));
    A->nzmax = nz;
    A->m = M;
    A->n = N;
    A->p = J;
    A->i = I;
    A->x = val;
    A->nz = nz;

    cs* compressed_A = cs_compress(A);
    get_triangular(compressed_A, true);

    return compressed_A;
}

void get_triangular(cs *A, bool lower)
{
    Int n = A->n;
    Int *Ap = A->p; Int *Ai = A->i; double *Ax = A->x;
    Int nz = 0;
    Int old_Ap = Ap[0];

    for (Int j = 0; j < n; j++)
    {
        for (Int p = old_Ap; p < Ap[j+1]; p++)
        {
            if ((lower && Ai[p] >= j) || (!lower && Ai[p] <= j))
            {
                Ai[nz] = Ai[p];
                Ax[nz] = Ax[p];
                nz++;
            }
        }
        old_Ap = Ap[j+1];
        Ap[j+1] = nz;
    }
}

} // end namespace Mongoose