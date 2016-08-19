#include "mongoose_sanitize.hpp"
#include "mongoose_interop.hpp"
#include <iostream>

using namespace std;

namespace Mongoose
{

cs *sanitize_matrix(cs *compressed_A, bool symmetric_triangular)
{
    cs *temp;
    if (symmetric_triangular)
    {
        temp = mirror_triangular(compressed_A);
    }
    else
    {
        cs* A_transpose = cs_transpose(compressed_A, 1);
        temp = cs_add(compressed_A, A_transpose, 0.5, 0.5);
        cs_spfree(A_transpose);
    }

    csd* dmperm = cs_scc(temp);
    if (!dmperm)
    {
        cout << "Ran out of memory, aborting." << endl;
        cs_spfree(temp);
        return NULL;
    }
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
    csi *pinv = cs_pinv(dmperm->p, temp->n);
    if (!pinv)
    {
        cout << "Ran out of memory, aborting." << endl;
        SuiteSparse_free(pinv);
        cs_spfree(temp);
        cs_dfree(dmperm);
        return NULL;
    }

    cs *C = cs_permute(temp, pinv, dmperm->p, 1);
    SuiteSparse_free(pinv);
    cs_spfree(temp);

    if (!C)
    {
        cout << "Ran out of memory, aborting." << endl;
        cs_dfree(dmperm);
        return NULL;
    }

    cs *submatrix = cs_submat(C, dmperm->r[largest_scc], 
                                 dmperm->r[largest_scc+1]-1, 
                                 dmperm->r[largest_scc], 
                                 dmperm->r[largest_scc+1]-1);
    cs_spfree(C);
    cs_dfree(dmperm);

    if (!submatrix)
    {
        cout << "Ran out of memory, aborting." << endl;
        return NULL;
    }

    remove_diagonal(submatrix);

    cs *D = cs_transpose (submatrix, 1);
    cs_spfree(submatrix);
    submatrix = cs_transpose (D, 1);

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
    cs* C = cs_compress(B);
    cs_spfree(B);

    return C;
}

} // end namespace Mongoose