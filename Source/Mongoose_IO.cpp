/**
 * Simplified I/O functions for reading matrices and graphs
 *
 * For reading Matrix Market files into Mongoose, read_graph and read_matrix
 * are provided (depending on if a Graph class instance or CSparse matrix
 * instance is needed). The filename can be specified as either a const char*
 * (easier for C programmers) or std::string (easier from C++).
 */

#include "Mongoose_Internal.hpp"
#include "Mongoose_IO.hpp"
#include "Mongoose_Interop.hpp"
#include "Mongoose_Sanitize.hpp"
#include <cstdio>
#include <iostream>
extern "C" {
#include "mmio.h"
}
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

using namespace std;

namespace Mongoose
{

Graph *readGraph (const std::string &filename)
{
    return readGraph(filename.c_str());
}

cs *readMatrix (const std::string &filename, MM_typecode &matcode)
{
    return readMatrix(filename.c_str(), matcode);
}

Graph *readGraph (const char* filename)
{
    Logger::tic(IOTiming);
    LogInfo("Reading graph from file " << filename << "\n");

    MM_typecode matcode;
    cs* A = readMatrix(filename, matcode);
    if (!A)
    {
        LogError("Error reading matrix from file\n");
        return NULL;
    }
    cs *sanitized_A = sanitizeMatrix(A, mm_is_symmetric(matcode));
    cs_spfree(A);
    if (!sanitized_A) return NULL;
    Graph *G = CSparse3ToGraph(sanitized_A);
    if (!G)
    {
        LogError("Ran out of memory in Mongoose::readGraph\n");
        cs_spfree(sanitized_A);
        Logger::toc(IOTiming);
        return NULL;
    }
    sanitized_A->p = NULL;
    sanitized_A->i = NULL;
    sanitized_A->x = NULL;
    cs_spfree(sanitized_A);

    Logger::toc(IOTiming);

    return G;
}

cs *readMatrix (const char* filename, MM_typecode &matcode)
{
    LogInfo("Reading Matrix from " << filename << "\n");
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        LogError("Error: Cannot read file " << filename << "\n");
        return NULL;
    }

    LogInfo("Reading Matrix Market banner...");
    if (mm_read_banner(file, &matcode) != 0)
    {
        LogError("Error: Could not process Matrix Market banner\n");
        fclose(file);
        return NULL;
    }
    if (!mm_is_matrix(matcode) || !mm_is_sparse(matcode) || 
        mm_is_complex(matcode))
    {
        LogError("Error: Unsupported matrix format - Must be real and sparse\n");
        fclose(file);
        return NULL;
    }

    Int M, N, nz;
    int ret_code;
    if ((ret_code = mm_read_mtx_crd_size(file, &M, &N, &nz)) !=0)
    {
        LogError("Error: Could not parse matrix dimension and size.\n");
        fclose(file);
        return NULL;
    }
    if (M != N)
    {
        LogError("Error: Matrix must be square.\n");
        fclose(file);
        return NULL;
    }
    
    LogInfo("Reading matrix data...\n");
    Int *I = (Int *) SuiteSparse_malloc(nz, sizeof(Int));
    Int *J = (Int *) SuiteSparse_malloc(nz, sizeof(Int));
    double *val = (double *) SuiteSparse_malloc(nz, sizeof(double));

    if (!I || !J || !val)
    {
        LogError("Error: Ran out of memory in Mongoose::readMatrix\n");
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
        LogError("Error: Ran out of memory in Mongoose::readMatrix\n");
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
    
    LogInfo("Compressing matrix from triplet to CSC format...\n");
    cs* compressed_A = cs_compress(A);
    cs_spfree(A);
    if (!compressed_A)
    {
        LogError("Error: Ran out of memory in Mongoose::readMatrix\n");
        return NULL;
    }
    
    return compressed_A;
}

} // end namespace Mongoose
