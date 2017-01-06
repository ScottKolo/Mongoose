/**
 * Fundamental sparse matrix operations.
 *
 * A subset of the CSparse library is used for its sparse matrix data
 * structure and efficient fundamental matrix operations, such as adding, 
 * permuting, and finding strongly connected components.
 */

#ifndef Mongoose_CSparse_hpp
#define Mongoose_CSparse_hpp

#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#ifdef MATLAB_MEX_FILE
#include "mex.h"
#endif

#ifdef MATLAB_MEX_FILE
#undef csi
#define csi mwSignedIndex
#endif
#ifndef csi
#define csi ptrdiff_t
#endif

/* CSparse Macros */
#ifndef CS_CSC
#define CS_CSC(A) (A && (A->nz == -1))
#endif
#ifndef CS_TRIPLET
#define CS_TRIPLET(A) (A && (A->nz >= 0))
#endif
#ifndef CS_MAX
#define CS_MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef CS_MIN
#define CS_MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef CS_FLIP
#define CS_FLIP(i) (-(i)-2)
#endif
#ifndef CS_UNFLIP
#define CS_UNFLIP(i) (((i) < 0) ? CS_FLIP(i) : (i))
#endif
#ifndef CS_MARKED
#define CS_MARKED(w,j) (w [j] < 0)
#endif
#ifndef CS_MARK
#define CS_MARK(w,j) { w [j] = CS_FLIP (w [j]) ; }
#endif

namespace Mongoose
{

/* --- primary CSparse routines and data structures ------------------------- */
typedef struct cs_sparse    /* matrix in compressed-column or triplet form */
{
    csi nzmax ;     /* maximum number of entries */
    csi m ;         /* number of rows */
    csi n ;         /* number of columns */
    csi *p ;        /* column pointers (size n+1) or col indices (size nzmax) */
    csi *i ;        /* row indices, size nzmax */
    double *x ;     /* numerical values, size nzmax */
    csi nz ;        /* # of entries in triplet matrix, -1 for compressed-col */
} cs ;

typedef struct cs_dmperm_results    /* cs_dmperm or cs_scc output */
{
    csi *p ;        /* size m, row permutation */
    csi *q ;        /* size n, column permutation */
    csi *r ;        /* size nb+1, block k is rows r[k] to r[k+1]-1 in A(p,q) */
    csi *s ;        /* size nb+1, block k is cols s[k] to s[k+1]-1 in A(p,q) */
    csi nb ;        /* # of blocks in fine dmperm decomposition */
    csi rr [5] ;    /* coarse row decomposition */
    csi cc [5] ;    /* coarse column decomposition */
} csd ;

cs *cs_add (const cs *A, const cs *B, double alpha, double beta) ;
cs *cs_transpose (const cs *A, csi values) ;

csi cs_entry (cs *T, csi i, csi j, double x) ;
cs *cs_compress (const cs *T) ;

cs *cs_spalloc (csi m, csi n, csi nzmax, csi values, csi triplet) ;
csi cs_sprealloc (cs *A, csi nzmax);
cs *cs_spfree (cs *A) ;
csd *cs_dfree (csd *D);

csd *cs_scc (cs *A) ;
cs *cs_submat(const cs *A,const csi i1, const csi i2,const csi j1,const csi j2);
cs *cs_permute (const cs *A, const csi *pinv, const csi *q, csi values);
csi *cs_pinv (csi const *p, csi n);

} // end namespace Mongoose

#endif