/**
 * Fundamental sparse matrix operations.
 *
 * A subset of the CSparse library is used for its sparse matrix data
 * structure and efficient fundamental matrix operations, such as adding, 
 * permuting, and finding strongly connected components.  This version
 * uses the same integer (csi or SuiteSparse_long) as the Int in Mongoose.
 */

#pragma once

#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include "Mongoose_Internal.hpp"
#ifdef MATLAB_MEX_FILE
#include "mex.h"
#endif

/* same as Int in Mongoose */
typedef SuiteSparse_long csi ;

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

cs *cs_add (const cs *A, const cs *B, double alpha, double beta) ;
cs *cs_transpose (const cs *A, csi values) ;

cs *cs_compress (const cs *T) ;

cs *cs_spalloc (csi m, csi n, csi nzmax, csi values, csi triplet) ;
cs *cs_spfree (cs *A) ;

} // end namespace Mongoose

