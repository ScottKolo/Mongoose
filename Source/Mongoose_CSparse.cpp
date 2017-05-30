/**
 * Fundamental sparse matrix operations.
 *
 * A subset of the CSparse library is used for its sparse matrix data
 * structure and efficient fundamental matrix operations, such as adding, 
 * permuting, and finding strongly connected components.
 */
#include "Mongoose_Internal.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_CSparse.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

csi *cs_counts (const cs *A, const csi *parent, const csi *post, csi ata);
double cs_cumsum (csi *p, csi *c, csi n);
csi cs_scatter (const cs *A, csi j, double beta, csi *w, double *x, csi mark,
                cs *C, csi nz);
cs *cs_done (cs *C, void *w, void *x, csi ok);
csd *cs_dalloc (csi m, csi n);
cs *cs_spfree (cs *A);
csd *cs_ddone (csd *D, cs *C, void *w, csi ok);
csi cs_dfs (csi j, cs *G, csi top, csi *xi, csi *pstack, const csi *pinv);

/**
 * C = A'
 *
 * @details Given a CSparse matrix @p A, cs_transpose returns a new matrix that 
 * is the transpose of @p A. In other words, C(j,i) = A(i,j).
 * The second parameter, @p values, allows for the copying of the A->x array.
 * If @p values = 0, the A->x array is ignored, and C->x is left NULL. If 
 * @p values != 0, then the A->x array is copied (transposed) to C->x.
 * 
 * @code
 * cs *C = cs_transpose(A, 0); // C is A' but C->x = NULL
 * cs *D = cs_transpose(A, 1); // D is A' and C->x is transpose of A->x
 * @endcode
 * 
 * @param A Matrix to be transposed
 * @param values 1 to transpose the values A->x, 0 to ignore
 * @return A transposed version of A
 * @note Allocates memory for the returned matrix, but frees on error.
 */
cs *cs_transpose (const cs *A, csi values)
{
    csi p, q, j, *Cp, *Ci, n, m, *Ap, *Ai, *w;
    double *Cx, *Ax;
    cs *C;
    if (!CS_CSC (A)) return (NULL);     /* check inputs */
    m = A->m; n = A->n; Ap = A->p; Ai = A->i; Ax = A->x;
    C = (cs*) cs_spalloc (n, m, Ap [n], values && Ax, 0);  /* allocate result */
    w = (csi*) SuiteSparse_calloc (m, sizeof (csi));       /* get workspace */
    if (!C || !w) return (cs_done (C, w, NULL, 0));        /* out of memory */
    Cp = C->p; Ci = C->i; Cx = C->x;
    for (p = 0; p < Ap [n]; p++) w [Ai [p]]++;             /* row counts */
    cs_cumsum (Cp, w, m);                                  /* row pointers */
    for (j = 0; j < n; j++)
    {
        for (p = Ap [j]; p < Ap [j+1]; p++)
        {
            Ci [q = w [Ai [p]]++] = j;  /* place A(i,j) as entry C(j,i) */
            if (Cx) Cx [q] = Ax [p];
        }
    }
    return (cs_done (C, w, NULL, 1));   /* success; release w and return C */
}

/**
 * C = alpha*A + beta*B
 *
 * @details Given two CSparse matrices @p A and @p B, and scaling constants 
 * @p alpha and @p beta, cs_add returns a new matrix such that 
 * C = alpha*A + beta*B. In other words, C(i,j) = alpha*A(i,j) + beta*B(i,j).
 * 
 * @code
 * cs *C = cs_add(A, B, 1, 1);     // C = A + B
 * cs *D = cs_add(A, B, 0.5, 0.5); // C = 0.5*A + 0.5*B
 * cs *E = cs_add(A, B, 1, 0);     // C = A
 * @endcode
 * 
 * @param A Matrix to be added to B.
 * @param B Matrix to be added to A
 * @param alpha Scalar (double) value to scale all elements of A matrix by
 * @param beta Scalar (double) value to scale all elements of B matrix by
 * @return A matrix such that C = alpha*A + beta*B
 * @note Allocates memory for the returned matrix, but frees on error.
 */
cs *cs_add (const cs *A, const cs *B, double alpha, double beta)
{
    csi p, j, nz = 0, anz, *Cp, *Ci, *Bp, m, n, bnz, *w, values;
    double *x, *Bx, *Cx;
    cs *C;
    if (!CS_CSC (A) || !CS_CSC (B)) /* check inputs */
    {
        LogError("Error: Cannot add uncompressed matrices");
        return (NULL);
    }
    if (A->m != B->m || A->n != B->n) return (NULL);
    m = A->m; anz = A->p [A->n];
    n = B->n; Bp = B->p; Bx = B->x; bnz = Bp [n];
    w = (csi*) SuiteSparse_calloc (m, sizeof (csi));               /* get workspace */
    values = (A->x != NULL) && (Bx != NULL);
    x = values ? (double*) SuiteSparse_malloc (m, sizeof (double)) : NULL;     /* get workspace */
    C = cs_spalloc (m, n, anz + bnz, values, 0);            /* allocate result*/
    if (!C || !w || (values && !x)) return (cs_done (C, w, x, 0));
    Cp = C->p; Ci = C->i; Cx = C->x;
    for (j = 0; j < n; j++)
    {
        Cp [j] = nz;                    /* column j of C starts here */
        nz = cs_scatter (A, j, alpha, w, x, j+1, C, nz);    /* alpha*A(:,j)*/
        nz = cs_scatter (B, j, beta, w, x, j+1, C, nz);     /* beta*B(:,j) */
        if (values) for (p = Cp [j]; p < nz; p++) Cx [p] = x [Ci [p]];
    }
    Cp [n] = nz;                        /* finalize the last column of C */
    return ((cs*) cs_done (C, w, x, 1));      /* success; release workspace, return C */
}

/**
 * C = compressed-column form of a triplet matrix T
 *
 * Given a CSparse matrix @p T in triplet form, cs_compress returns
 * the same matrix in compressed sparse column (CSC) format.
 * 
 * A triplet form matrix is a simple listing of the nonzeros in the matrix and
 * their row, column coordinates. For example, the 2x2 identity matrix would be
 * specified with the following triplets in (row, column, value) format:
 * (1, 1, 1) and (2, 2, 1).
 * 
 * @param T Matrix in triplet form to be compressed
 * @return A compressed sparse column (CSC) format matrix representing the 
 * triplet form matrix T passed in.
 * @note Uses an O(n) workspace w, and frees on error.
 */
cs *cs_compress (const cs *T)
{
    csi m, n, nz, k, *Cp, *Ci, *w, *Ti, *Tj;
    double *Cx, *Tx;
    cs *C;
    if (!CS_TRIPLET (T)) return (NULL);                 /* check inputs */
    m = T->m; n = T->n; Ti = T->i; Tj = T->p; Tx = T->x; nz = T->nz;
    C = cs_spalloc (m, n, nz, Tx != NULL, 0);           /* allocate result */
    w = (csi*) SuiteSparse_calloc (n, sizeof (csi));    /* get workspace */
    if (!C || !w) return (cs_done (C, w, NULL, 0));     /* out of memory */
    Cp = C->p; Ci = C->i; Cx = C->x;
    for (k = 0; k < nz; k++) w [Tj [k]]++;              /* column counts */
    cs_cumsum (Cp, w, n);                               /* column pointers */
    for (k = 0; k < nz; k++)
    {
        csi p = w [Tj [k]]++;
        Ci [p] = Ti [k];     /* A(i,j) is the pth entry in C */
        if (Cx) Cx [p] = Tx [k];
    }
    return (cs_done (C, w, NULL, 1));       /* success; release w and return C */
}

/**
 * p [0..n] = cumulative sum of c [0..n-1], and then copy p [0..n-1]
 * into c
 * 
 * @param p A vector of size @p n to be summed. On return, p is overwritten such
 * that p[i] contains the cumulative sum of p[0..i] that was passed in.
 * @param c A vector of size @p n used as a workspace. On return, c[i] contains
 * the cumulative sum of p[0..i].
 * @param n The dimension of @p p and @p c
 * @return The cumulative sum of c[0..n-1]
 */
double cs_cumsum (csi *p, csi *c, csi n)
{
    csi i, nz = 0;
    double nz2 = 0;
    if (!p || !c) return (-1);      /* check inputs */
    for (i = 0; i < n; i++)
    {
        p [i] = nz;
        nz += c [i];
        nz2 += c [i];               /* also in double to avoid csi overflow */
        c [i] = p [i];              /* also copy p[0..n-1] back into c[0..n-1]*/
    }
    p [n] = nz;
    return (nz2);                   /* return sum (c [0..n-1]) */
}

//-----------------------------------------------------------------------------
// x = x + beta * A(:,j), where x is a dense vector and A(:,j) is sparse
//-----------------------------------------------------------------------------
csi cs_scatter (const cs *A, csi j, double beta, csi *w, double *x, csi mark,
                cs *C, csi nz)
{
    csi p, *Ap, *Ai, *Ci;
    double *Ax;
    if (!CS_CSC (A) || !w || !CS_CSC (C)) return (-1);      /* check inputs */
    Ap = A->p; Ai = A->i; Ax = A->x; Ci = C->i;
    for (p = Ap [j]; p < Ap [j+1]; p++)
    {
        csi i = Ai [p];                         /* A(i,j) is nonzero */
        if (w [i] < mark)
        {
            w [i] = mark;                       /* i is new entry in column j */
            Ci [nz++] = i;                      /* add i to pattern of C(:,j) */
            if (x) x [i] = beta * Ax [p];       /* x(i) = beta*A(i,j) */
        }
        else if (x) x [i] += beta * Ax [p];     /* i exists in C(:,j) already */
    }
    return (nz);
}

//-----------------------------------------------------------------------------
// utility functions
//-----------------------------------------------------------------------------
/* allocate a sparse matrix (triplet form or compressed-column form) */
cs *cs_spalloc (csi m, csi n, csi nzmax, csi values, csi triplet)
{
    cs *A = (cs*) SuiteSparse_calloc (1, sizeof (cs));     /* allocate the cs struct */
    if (!A) return (NULL);                  /* out of memory */
    A->m = m;                               /* define dimensions and nzmax */
    A->n = n;
    A->nzmax = nzmax = std::max(nzmax, 1L);
    A->nz = triplet ? 0 : -1;               /* allocate triplet or comp.col */
    A->p = (csi*) SuiteSparse_malloc (triplet ? nzmax : n+1, sizeof (csi));
    A->i = (csi*) SuiteSparse_malloc (nzmax, sizeof (csi));
    A->x =
        values ? (double*) SuiteSparse_malloc (nzmax, sizeof (double)) : NULL;
    return ((!A->p || !A->i || (values && !A->x)) ? cs_spfree (A) : A);
}

/**
 * Change the max number of entries in a sparse matrix
 *
 * Given a CSparse matrix @p A in compressed sparse column format,
 * cs_sprealloc (i.e. Sparse Matrix Reallocation) will modify the matrix data
 * structure to allow for a new maximum capacity.
 * 
 * @param A The sparse matrix (in compressed sparse column format) to be 
 * reallocated.
 * @param nzmax The new maximum number of nonzeros the matrix should be able to
 * accomodate.
 * @return An error code ok valued 1 for success and 0 for failure.
 */
csi cs_sprealloc (cs *A, csi nzmax)
{
    int ok, oki, okj = 1, okx = 1;
    if (!A) return (0);
    if (nzmax <= 0) nzmax = (CS_CSC (A)) ? (A->p [A->n]) : A->nz;
    A->i = (csi*) SuiteSparse_realloc (A->nzmax, nzmax, sizeof (csi), A->i, &oki);
    if (CS_TRIPLET (A))
    {
        A->p = (csi*) SuiteSparse_realloc (A->nzmax, nzmax, sizeof(csi), 
                                           A->p, &okj);
    }
    if (A->x)
    {
        A->x = (double*) SuiteSparse_realloc (A->nzmax, nzmax, sizeof(double), 
                                              A->x, &okx);
    }
    ok = (oki && okj && okx);
    if (ok)
    {
        A->nzmax = nzmax;
    }
    return (ok);
}

/* allocate a cs_dmperm or cs_scc result */
csd *cs_dalloc (csi m, csi n)
{
    csd *D;
    D = (csd *)SuiteSparse_calloc (1, sizeof (csd));
    if (!D) return (NULL);
    D->p = (csi *)SuiteSparse_malloc (m, sizeof (csi));
    D->r = (csi *)SuiteSparse_malloc (m+6, sizeof (csi));
    D->q = (csi *)SuiteSparse_malloc (n, sizeof (csi));
    D->s = (csi *)SuiteSparse_malloc (n+6, sizeof (csi));
    return ((!D->p || !D->r || !D->q || !D->s) ? cs_dfree (D) : D);
}

/* release a sparse matrix */
cs *cs_spfree (cs *A)
{
    if (!A) return (NULL);      /* do nothing if A already NULL */
    SuiteSparse_free (A->p);
    SuiteSparse_free (A->i);
    SuiteSparse_free (A->x);
    return ((cs *) SuiteSparse_free (A));    /* release the cs struct and return NULL */
}

/* free a cs_dmperm or cs_scc result */
csd *cs_dfree (csd *D)
{
    if (!D) return (NULL);      /* do nothing if D already NULL */
    SuiteSparse_free (D->p);
    SuiteSparse_free (D->q);
    SuiteSparse_free (D->r);
    SuiteSparse_free (D->s);
    return ((csd *) SuiteSparse_free (D));   /* free the csd struct and return NULL */
}

/* release workspace and return a sparse matrix result */
cs *cs_done (cs *C, void *w, void *x, csi ok)
{
    SuiteSparse_free (w);                        /* release workspace */
    SuiteSparse_free (x);
    return (ok ? C : cs_spfree (C));    /* return result if OK, else release it */
}

/* free workspace and return a csd result */
csd *cs_ddone (csd *D, cs *C, void *w, csi ok)
{
    cs_spfree (C);                      /* free temporary matrix */
    SuiteSparse_free (w);                        /* free workspace */
    return (ok ? D : cs_dfree (D));     /* return result if OK, else free it */
}

/* depth-first-search of the graph of a matrix, starting at node j */
csi cs_dfs (csi j, cs *G, csi top, csi *xi, csi *pstack, const csi *pinv)
{
    csi i, p, p2, done, jnew, head = 0, *Gp, *Gi;
    if (!CS_CSC (G) || !xi || !pstack) return (-1);     /* check inputs */
    Gp = G->p; Gi = G->i;
    xi [0] = j;                 /* initialize the recursion stack */
    while (head >= 0)
    {
        j = xi [head];          /* get j from the top of the recursion stack */
        jnew = pinv ? (pinv [j]) : j;
        if (!CS_MARKED (Gp, j))
        {
            CS_MARK (Gp, j);        /* mark node j as visited */
            pstack [head] = (jnew < 0) ? 0 : CS_UNFLIP (Gp [jnew]);
        }
        done = 1;                   /* node j done if no unvisited neighbors */
        p2 = (jnew < 0) ? 0 : CS_UNFLIP (Gp [jnew+1]);
        for (p = pstack [head]; p < p2; p++)    /* examine all neighbors of j */
        {
            i = Gi [p];             /* consider neighbor node i */
            if (CS_MARKED (Gp, i)) continue;    /* skip visited node i */
            pstack [head] = p;      /* pause depth-first search of node j */
            xi [++head] = i;        /* start dfs at node i */
            done = 0;               /* node j is not done */
            break;                  /* break, to start dfs (i) */
        }
        if (done)               /* depth-first search at node j is done */
        {
            head--;             /* remove j from the recursion stack */
            xi [--top] = j;     /* and place in the output stack */
        }
    }
    return (top);
}

/* find the strongly connected components of a square matrix */
csd *cs_scc (cs *A)     /* matrix A temporarily modified, then restored */
{
    csi n, i, k, b, nb = 0, top, *xi, *pstack, *p, *r, *Ap, *ATp, *rcopy, *Blk;
    cs *AT;
    csd *D;
    if (!CS_CSC (A)) return (NULL);                 /* check inputs */
    n = A->n; Ap = A->p;
    D = cs_dalloc (n, 0);                           /* allocate result */
    AT = cs_transpose (A, 0);                       /* AT = A' */
    xi = (csi *)SuiteSparse_malloc (2*n+1, sizeof (csi));           /* get workspace */
    if (!D || !AT || !xi) return (cs_ddone (D, AT, xi, 0));
    Blk = xi; rcopy = pstack = xi + n;
    p = D->p; r = D->r; ATp = AT->p;
    top = n;
    for (i = 0; i < n; i++)     /* first dfs(A) to find finish times (xi) */
    {
        if (!CS_MARKED (Ap, i)) top = cs_dfs (i, A, top, xi, pstack, NULL);
    }
    for (i = 0; i < n; i++) CS_MARK (Ap, i);    /* restore A; unmark all nodes*/
    top = n;
    nb = n;
    for (k = 0; k < n; k++)     /* dfs(A') to find strongly connnected comp */
    {
        i = xi [k];             /* get i in reverse order of finish times */
        if (CS_MARKED (ATp, i)) continue;   /* skip node i if already ordered */
        r [nb--] = top;         /* node i is the start of a component in p */
        top = cs_dfs (i, AT, top, p, pstack, NULL);
    }
    r [nb] = 0;                 /* first block starts at zero; shift r up */
    for (k = nb; k <= n; k++) r [k-nb] = r [k];
    D->nb = nb = n-nb;          /* nb = # of strongly connected components */
    for (b = 0; b < nb; b++)    /* sort each block in natural order */
    {
        for (k = r [b]; k < r [b+1]; k++) Blk [p [k]] = b;
    }
    for (b = 0; b <= nb; b++) rcopy [b] = r [b];
    for (i = 0; i < n; i++) p [rcopy [Blk [i]]++] = i;
    return (cs_ddone (D, AT, xi, 1));
}

/* C = A(i1:i2;j1:j2) */
/* Written by Mohsen Aznaveh */
cs *cs_submat(const cs *A, const csi i1, const csi i2, 
    const csi j1,const csi j2)
{
    csi pA, pC, j, anz, *Cp, *Ci, m, n, values;
    double  *Cx;
    cs *C;
    if (!CS_CSC (A)) /* check inputs */
    {
        return (NULL);
    }
    anz = A->p [A->n];
    m = i2-i1+1;
    n = j2-j1+1;
    values = (A->x != NULL);
    C = cs_spalloc (m, n, anz, values, 0);         /* allocate result */
    if (!C) return NULL;
    Cp = C->p; Ci = C->i; Cx = C->x;
    
    /* dicing the matrix */
    pC = 0;
    for (j = j1; j <= j2; j++)   /*searching through columns between j1 and j2*/
    {
        Cp[j-j1] = pC;
        for (pA = A->p[j]; pA < A->p [j+1]; pA++)   /* finding the rows */
        {
            if (A->i[pA] >= i1 && A->i[pA] <= i2)
            {
                Ci[pC] = A->i[pA]-i1;
                if (Cx && A->x) Cx[pC++] = A->x[pA];
            }
        }
    }
    Cp [n] = pC;                       /* finalize the last column of C */
    if(!cs_sprealloc (C, 0)) /* remove extra space from C */
    {
        cs_spfree(C);
        return NULL;
    }
    return (C);
}

/* C = A(p,q) where p and q are permutations of 0..m-1 and 0..n-1. */
cs *cs_permute (const cs *A, const csi *pinv, const csi *q, csi values)
{
    csi t, j, k, nz = 0, m, n, *Ap, *Ai, *Cp, *Ci;
    double *Cx, *Ax;
    cs *C;
    if (!CS_CSC (A)) return (NULL);     /* check inputs */
    m = A->m; n = A->n; Ap = A->p; Ai = A->i; Ax = A->x;
    C = cs_spalloc (m, n, Ap [n], values && Ax != NULL, 0);   /* alloc result */
    if (!C) return (cs_done (C, NULL, NULL, 0));    /* out of memory */
    Cp = C->p; Ci = C->i; Cx = C->x;
    for (k = 0; k < n; k++)
    {
        Cp [k] = nz;                    /* column k of C is column q[k] of A */
        j = q ? (q [k]) : k;
        for (t = Ap [j]; t < Ap [j+1]; t++)
        {
            if (Cx && Ax) Cx [nz] = Ax [t];   /* row i of A is row pinv[i] of C */
            Ci [nz++] = pinv ? (pinv [Ai [t]]) : Ai [t];
        }
    }
    Cp [n] = nz;                        /* finalize the last column of C */
    return (cs_done (C, NULL, NULL, 1));
}

/* pinv = p', or p = pinv' */
csi *cs_pinv (csi const *p, csi n)
{
    csi k, *pinv;
    if (!p) return (NULL);                      /* p = NULL denotes identity */
    pinv = (csi *)SuiteSparse_malloc (n, sizeof (csi));   /* allocate result */
    if (!pinv) return (NULL);                   /* out of memory */
    for (k = 0; k < n; k++) pinv [p [k]] = k;   /* invert the permutation */
    return (pinv);                              /* return result */
}

} // end namespace Mongoose
