
#include "mongoose_io.hpp"
#include "mongoose_interop.hpp"
#include "mongoose_debug.hpp"
#include "mongoose_sanitize.hpp"
#include <cstdio>
#include <iostream>
extern "C" {
#include "mmio.h"
}

using namespace std;

namespace Mongoose
{

Graph *read_graph (const std::string filename)
{
    return read_graph(filename.c_str());
}

cs *read_matrix (const std::string filename)
{
    return read_matrix(filename.c_str());
}

Graph *read_graph (const char* filename)
{
    cs* A = read_matrix(filename);
    if (!A) return NULL;
    Graph *G = CSparse3ToGraph(A);
    if (!G)
    {
        cs_spfree(A);
        return NULL;
    }
    A->p = NULL;
    A->i = NULL;
    A->x = NULL;
    cs_spfree(A);
    return G;
}

cs *read_matrix (const char* filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        cout << "Error: Cannot find file " << filename << endl;
        return NULL;
    }

    MM_typecode matcode;
    if (mm_read_banner(file, &matcode) != 0)
    {
        cout << "Could not process Matrix Market banner." << endl;
        fclose(file);
        return NULL;
    }
    if (!mm_is_matrix(matcode) || !mm_is_sparse(matcode) || 
        mm_is_complex(matcode))
    {
        cout << 
            "Unsupported matrix format - Must be real and sparse"
             << endl;
        fclose(file);
        return NULL;
    }

    Int M, N, nz;
    int ret_code;
    if ((ret_code = mm_read_mtx_crd_size(file, &M, &N, &nz)) !=0)
    {
        cout << "Could not parse matrix dimension and size." << endl;
        fclose(file);
        return NULL;
    }
    if (M != N)
    {
        cout << "Matrix must be square." << endl;
        fclose(file);
        return NULL;
    }
    
    Int *I = (Int *) SuiteSparse_malloc(nz, sizeof(Int));
    Int *J = (Int *) SuiteSparse_malloc(nz, sizeof(Int));
    Weight *val = (Weight *) SuiteSparse_malloc(nz, sizeof(Weight));

    if (!I || !J || !val)
    {
        cout << "Ran out of memory, aborting." << endl;
        SuiteSparse_free(I);
        SuiteSparse_free(J);
        SuiteSparse_free(val);
        fclose(file);
        return NULL;
    }

    mm_read_mtx_crd_data(file, M, N, nz, I, J, val, matcode);
    fclose(file); // Close the file
    for (Int k = 0; k < nz; k++)
    {
        --I[k];
        --J[k];
        if (mm_is_pattern(matcode)) val[k] = 1;
    }

    cs *A = (cs *) SuiteSparse_malloc(1, sizeof(cs));
    if (!A)
    {
        cout << "Ran out of memory, aborting." << endl;
        SuiteSparse_free(I);
        SuiteSparse_free(J);
        SuiteSparse_free(val);
        return NULL;
    }

    A->nzmax = nz;
    A->m = M;
    A->n = N;
    A->p = J;
    A->i = I;
    A->x = val;
    A->nz = nz;
    
    cs* compressed_A = cs_compress(A);
    cs_spfree(A);
    if (!compressed_A)
    {
        cout << "Ran out of memory, aborting." << endl;
        return NULL;
    }
    cs *sanitized_A = sanitize_matrix(compressed_A, mm_is_symmetric(matcode));

    return sanitized_A;
}

} // end namespace Mongoose