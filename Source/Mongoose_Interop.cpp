#include "Mongoose_Internal.hpp"
#include "Mongoose_Interop.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

/* Configure a CSparse3 matrix from an existing Mongoose Graph. */
cs *GraphToCSparse3(Graph *G, bool copy)
{
    cs *A = (cs*) SuiteSparse_malloc(1, sizeof(cs));
    if (!A) return NULL;
    A->n = G->cs_n;
    A->m = G->cs_m;
    A->nz = G->cs_nz;
    A->nzmax = G->cs_nzmax;
    if (!copy)
    {
        A->p = (ptrdiff_t*) G->p;
        A->i = (ptrdiff_t*) G->i;
        A->x = G->x;
    }
    else
    {
        Int n = G->n;
        Int nz = G->nz;
        A->p = (ptrdiff_t*) SuiteSparse_malloc((n+1), sizeof(ptrdiff_t));
        A->i = (ptrdiff_t*) SuiteSparse_malloc(nz, sizeof(ptrdiff_t));
        A->x = (double*) SuiteSparse_malloc(nz, sizeof(double));
        if (!A->p || !A->i || !A->x)
        {
            cs_spfree(A);
            return NULL;
        }

        for (Int k = 0; k <= n; k++)
        {
            A->p[k] = (ptrdiff_t) G->p[k];
        }
        for (Int p = 0; p < nz; p++)
        {
            A->i[p] = (ptrdiff_t) G->i[p];
            A->x[p] = G->x[p];
        }
    }

    return A;
}

/* Create a new Mongoose Graph from an existing CSparse3 matrix. */
Graph *CSparse3ToGraph(cs *G, bool resetEW, bool resetNW)
{
    Graph *returner = static_cast<Graph*>(SuiteSparse_calloc(1, sizeof(Graph)));
    if (!returner) return NULL;

    /* Brain-transplant the graph to the new representation. */
    returner->cs_n = G->n;
    returner->cs_m = G->m;
    returner->cs_nz = G->nz;
    returner->cs_nzmax = G->nzmax;
    returner->n = MONGOOSE_MAX2(G->n, G->m);
    returner->nz = G->p[G->n];
    returner->p = (Int*) G->p;
    returner->i = (Int*) G->i;
    returner->x = G->x;

    /* Allocate edge weights if necessary. */
    bool attachEdgeWeights = false;
    if (!returner->x || resetEW)
    {
        Int nz = returner->nz;
        returner->x = (Weight*) SuiteSparse_malloc(nz, sizeof(Weight));
        attachEdgeWeights = true;
    }

    /* Allocate node weights if necessary. */
    bool attachNodeWeights = false;
    if (!returner->w || resetNW)
    {
        Int n = returner->n;
        returner->w = (Weight*) SuiteSparse_malloc(n, sizeof(Weight));
        attachNodeWeights = true;
    }

    /* If we failed to attach weights, free the graph and return. */
    if ((attachEdgeWeights && !returner->x) ||
        (attachNodeWeights && !returner->w))
    {
        /* Undo the brain transplant, free the Graph skeleton, and return. */
        returner->p = NULL;
        returner->i = NULL;
        returner->x = NULL;
        returner->~Graph();
        SuiteSparse_free(returner);
        return NULL;
    }

    if (attachEdgeWeights)
    {
        Int nz = returner->nz;
        for (Int p = 0; p < nz; p++) returner->x[p] = 1.0;
    }

    if (attachNodeWeights)
    {
        int n = returner->n;
        for (Int k = 0; k < n; k++) returner->w[k] = 1.0;
    }

    return returner;
}

} // end namespace Mongoose
