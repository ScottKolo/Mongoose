
#include "mongoose_io.hpp"
#include "mongoose_interop.hpp"
#include "mongoose_debug.hpp"
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
    cout << "Reading graph file " << filename << endl;
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
    return G;
}

cs *read_matrix (const char* filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        cout << "Error: Cannot find file " << filename << endl;
        cout << "Exiting..." << endl;
        cout << "File closed, returning NULL" << endl;
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
        mm_is_complex(matcode) || !mm_is_symmetric(matcode))
    {
        cout << 
            "Unsupported matrix format - Must be real, sparse, and symmetric"
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
    
    Int *I = (Int *) SuiteSparse_malloc(nz, sizeof(Int));
    Int *J = (Int *) SuiteSparse_malloc(nz, sizeof(Int));
    Weight *val = (Weight *) SuiteSparse_malloc(nz, sizeof(Weight));

    if (!I || !J || !val)
    {
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
    if (!A) return NULL;

    A->nzmax = nz;
    A->m = M;
    A->n = N;
    A->p = J;
    A->i = I;
    A->x = val;
    A->nz = nz;
    
    cs* compressed_A = cs_compress(A);
    if (!compressed_A) return NULL;
    remove_diagonal(compressed_A);
    compressed_A = mirror_triangular(compressed_A);
    csd* dmperm = cs_scc (compressed_A);
    if (!dmperm) return NULL;
    int largest_size = 0;
    int largest_scc = 0;
    int scc_size = 0;
    for (int i = 0; i < dmperm->nb; i++)
    {
        scc_size = dmperm->r[i+1]-1 - dmperm->r[i] ;
        if (scc_size > largest_size)
        {
            largest_size = scc_size;
            largest_scc = i;
        }
    }
    
    // Get submatrix from dmperm
    csi *pinv = cs_pinv(dmperm->p, compressed_A->n);
    if (!pinv) return NULL;
    cs *C = cs_permute(compressed_A, pinv, dmperm->p, 1);
    if (!C) return NULL;
    cs *submatrix = cs_submat(C, dmperm->r[largest_scc], 
                                dmperm->r[largest_scc+1]-1, 
                                dmperm->r[largest_scc], 
                                dmperm->r[largest_scc+1]-1) ;
    if (!submatrix) return NULL;
    return submatrix;
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

void remove_diagonal(cs *A)
{
    Int n = A->n;
    Int *Ap = A->p; Int *Ai = A->i; double *Ax = A->x;
    Int nz = 0;
    Int old_Ap = Ap[0];

    for (Int j = 0; j < n; j++)
    {
        for (Int p = old_Ap; p < Ap[j+1]; p++)
        {
            if (Ai[p] != j)
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

// Requires A to be a triangular matrix with no diagonal.
cs *mirror_triangular(cs *A)
{
    Int A_n = A->n; Int A_nz = A->p[A_n];
    Int B_nz = 2*A_nz;

    cs *B = cs_spalloc (A_n, A_n, B_nz, 1, 1);
    if (!B) return NULL;

    Int *Ap = A->p; Int *Ai = A->i; double *Ax = A->x;
    Int *Bp = B->p; Int *Bi = B->i; double *Bx = B->x;
    Int nz = 0;

    for (Int j = 0; j < A_n; j++)
    {
        for (Int p = Ap[j]; p < Ap[j+1]; p++)
        {
            Bi[nz] = Ai[p];
            Bp[nz] = j;
            Bx[nz] = Ax[p];
            nz++;
            Bi[nz] = j;
            Bp[nz] = Ai[p];
            Bx[nz] = Ax[p];
            nz++;
        }
    }
    B->nz = nz;

    return cs_compress(B);
}

} // end namespace Mongoose