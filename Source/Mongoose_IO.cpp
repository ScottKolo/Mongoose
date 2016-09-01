/**
 * @file mongoose_io.cpp
 * @author Scott Kolodziej
 * @date 13 Jun 2016
 * @brief Simplified I/O functions for reading matrices and graphs
 *
 * For reading Matrix Market files into Mongoose, read_graph and read_matrix
 * are provided (depending on if a Graph class instance or CSparse matrix
 * instance is needed). The filename can be specified as either a const char*
 * (easier for C programmers) or std::string (easier from C++).
 */

#include "Mongoose_IO.hpp"
#include "Mongoose_Interop.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Sanitize.hpp"
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

cs *read_matrix (const std::string filename, MM_typecode &matcode)
{
    return read_matrix(filename.c_str(), matcode);
}

Graph *read_graph (const char* filename)
{
    MM_typecode matcode;
    cs* A = read_matrix(filename, matcode);
    if (!A) return NULL;
    cs *sanitized_A = sanitize_matrix(A, mm_is_symmetric(matcode));
    cs_spfree(A);
    if (!sanitized_A) return NULL;
    Graph *G = CSparse3ToGraph(sanitized_A);
    if (!G)
    {
        cs_spfree(sanitized_A);
        return NULL;
    }
    sanitized_A->p = NULL;
    sanitized_A->i = NULL;
    sanitized_A->x = NULL;
    cs_spfree(sanitized_A);
    return G;
}

cs *read_matrix (const char* filename, MM_typecode &matcode)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        cerr << "Error: Cannot find file " << filename << endl;
        return NULL;
    }

    //MM_typecode &matcode;
    if (mm_read_banner(file, &matcode) != 0)
    {
        cerr << "Could not process Matrix Market banner." << endl;
        fclose(file);
        return NULL;
    }
    if (!mm_is_matrix(matcode) || !mm_is_sparse(matcode) || 
        mm_is_complex(matcode))
    {
        cerr << 
            "Unsupported matrix format - Must be real and sparse"
             << endl;
        fclose(file);
        return NULL;
    }

    Int M, N, nz;
    int ret_code;
    if ((ret_code = mm_read_mtx_crd_size(file, &M, &N, &nz)) !=0)
    {
        cerr << "Could not parse matrix dimension and size." << endl;
        fclose(file);
        return NULL;
    }
    if (M != N)
    {
        cerr << "Matrix must be square." << endl;
        fclose(file);
        return NULL;
    }
    
    Int *I = (Int *) SuiteSparse_malloc(nz, sizeof(Int));
    Int *J = (Int *) SuiteSparse_malloc(nz, sizeof(Int));
    Weight *val = (Weight *) SuiteSparse_malloc(nz, sizeof(Weight));

    if (!I || !J || !val)
    {
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
        return NULL;
    }
    
    return compressed_A;
}

} // end namespace Mongoose