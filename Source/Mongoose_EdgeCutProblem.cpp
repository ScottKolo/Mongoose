/* ========================================================================== */
/* === Source/Mongoose_EdgeCutProblem.cpp =================================== */
/* ========================================================================== */

/* -----------------------------------------------------------------------------
 * Mongoose Graph Partitioning Library  Copyright (C) 2017-2018,
 * Scott P. Kolodziej, Nuri S. Yeralan, Timothy A. Davis, William W. Hager
 * Mongoose is licensed under Version 3 of the GNU General Public License.
 * Mongoose is also available under other licenses; contact authors for details.
 * -------------------------------------------------------------------------- */

#include "Mongoose_EdgeCutProblem.hpp"

#include <algorithm>
#include <new>

namespace Mongoose
{

/* Constructor & Destructor */
EdgeCutProblem::EdgeCutProblem()
{
    n = nz = 0;
    p      = NULL;
    i      = NULL;
    x      = NULL;
    w      = NULL;
    X      = 0.0;
    W      = 0.0;
    H      = 0.0;

    partition      = NULL;
    vertexGains    = NULL;
    externalDegree = NULL;
    bhIndex        = NULL;
    bhHeap[0] = bhHeap[1] = NULL;
    bhSize[0] = bhSize[1] = 0;

    heuCost   = 0.0;
    cutCost   = 0.0;
    W0        = 0.0;
    W1        = 0.0;
    imbalance = 0.0;

    parent      = NULL;
    clevel      = 0;
    cn          = 0;
    matching    = NULL;
    matchmap    = NULL;
    invmatchmap = NULL;
    matchtype   = NULL;

    markArray = NULL;
    markValue = 1;
}

EdgeCutProblem *EdgeCutProblem::create(const Graph *graph)
{
    if (!graph)
        return NULL;

    void *memoryLocation = SuiteSparse_malloc(1, sizeof(EdgeCutProblem));
    if (!memoryLocation)
        return NULL;

    // Placement new
    EdgeCutProblem *problem = new (memoryLocation) EdgeCutProblem();

    problem->shallow_p = (graph->p != NULL);
    problem->shallow_i = (graph->i != NULL);
    problem->shallow_x = (graph->x != NULL);
    problem->shallow_w = (graph->w != NULL);

    size_t n = static_cast<size_t>(graph->n);
    problem->n = graph->n;

    size_t nz = static_cast<size_t>(graph->nz);
    problem->nz = graph->nz;

    problem->p = (problem->shallow_p)
                   ? graph->p
                   : (Int *)SuiteSparse_calloc(n + 1, sizeof(Int));
    problem->i
        = (problem->shallow_i) ? graph->i : (Int *)SuiteSparse_malloc(nz, sizeof(Int));
    problem->x = graph->x;
    problem->w = graph->w;
    problem->X = 0.0;
    problem->W = 0.0;
    problem->H = 0.0;
    if (!problem->p || !problem->i)
    {
        problem->~EdgeCutProblem();
        return NULL;
    }

    problem->partition      = (bool *)SuiteSparse_malloc(n, sizeof(bool));
    problem->vertexGains    = (double *)SuiteSparse_malloc(n, sizeof(double));
    problem->externalDegree = (Int *)SuiteSparse_calloc(n, sizeof(Int));
    problem->bhIndex        = (Int *)SuiteSparse_calloc(n, sizeof(Int));
    problem->bhHeap[0]      = (Int *)SuiteSparse_malloc(n, sizeof(Int));
    problem->bhHeap[1]      = (Int *)SuiteSparse_malloc(n, sizeof(Int));
    problem->bhSize[0] = problem->bhSize[1] = 0;
    if (!problem->partition || !problem->vertexGains || !problem->externalDegree
        || !problem->bhIndex || !problem->bhHeap[0] || !problem->bhHeap[1])
    {
        problem->~EdgeCutProblem();
        return NULL;
    }

    problem->heuCost   = 0.0;
    problem->cutCost   = 0.0;
    problem->W0        = 0.0;
    problem->W1        = 0.0;
    problem->imbalance = 0.0;

    problem->parent      = NULL;
    problem->clevel      = 0;
    problem->cn          = 0;
    problem->matching    = (Int *)SuiteSparse_calloc(n, sizeof(Int));
    problem->matchmap    = (Int *)SuiteSparse_malloc(n, sizeof(Int));
    problem->invmatchmap = (Int *)SuiteSparse_malloc(n, sizeof(Int));
    problem->matchtype   = (Int *)SuiteSparse_malloc(n, sizeof(Int));
    problem->markArray   = (Int *)SuiteSparse_calloc(n, sizeof(Int));
    problem->markValue   = 1;
    problem->singleton   = -1;
    if (!problem->matching || !problem->matchmap || !problem->invmatchmap
        || !problem->markArray || !problem->matchtype)
    {
        problem->~EdgeCutProblem();
        return NULL;
    }

    problem->initialized = false;

    return problem;
}

EdgeCutProblem *EdgeCutProblem::create(EdgeCutProblem *_parent)
{
    if (!_parent)
        return NULL;

    void *memoryLocation = SuiteSparse_malloc(1, sizeof(EdgeCutProblem));
    if (!memoryLocation)
        return NULL;

    // Placement new
    EdgeCutProblem *problem = new (memoryLocation) EdgeCutProblem();

    problem->n  = _parent->cn;
    problem->nz = _parent->nz;

    problem->x = (double *)SuiteSparse_malloc(_parent->nz, sizeof(double));
    problem->w = (double *)SuiteSparse_malloc(_parent->cn, sizeof(double));

    if (!problem->x || !problem->w)
    {
        problem->~EdgeCutProblem();
        return NULL;
    }

    problem->W      = _parent->W;
    problem->parent = _parent;
    problem->clevel = problem->parent->clevel + 1;

    return problem;
}

EdgeCutProblem::~EdgeCutProblem()
{
    p = (shallow_p) ? NULL : (Int *)SuiteSparse_free(p);
    i = (shallow_i) ? NULL : (Int *)SuiteSparse_free(i);
    x = (shallow_x) ? NULL : (double *)SuiteSparse_free(x);
    w = (shallow_w) ? NULL : (double *)SuiteSparse_free(w);

    partition      = (bool *)SuiteSparse_free(partition);
    vertexGains    = (double *)SuiteSparse_free(vertexGains);
    externalDegree = (Int *)SuiteSparse_free(externalDegree);
    bhIndex        = (Int *)SuiteSparse_free(bhIndex);
    bhHeap[0]      = (Int *)SuiteSparse_free(bhHeap[0]);
    bhHeap[1]      = (Int *)SuiteSparse_free(bhHeap[1]);
    matching       = (Int *)SuiteSparse_free(matching);
    matchmap       = (Int *)SuiteSparse_free(matchmap);
    invmatchmap    = (Int *)SuiteSparse_free(invmatchmap);
    matchtype      = (Int *)SuiteSparse_free(matchtype);

    markArray = (Int *)SuiteSparse_free(markArray);

    SuiteSparse_free(this);
}

/* Initialize a top level graph with a a set of options. */
void EdgeCutProblem::initialize(const EdgeCut_Options *options)
{
    (void)options; // Unused variable

    if (initialized)
    {
        // Graph has been previously initialized. We need to clear some extra
        // data structures to be able to reuse it.

        X = 0.0;
        W = 0.0;
        H = 0.0;

        bhSize[0] = bhSize[1] = 0;

        heuCost   = 0.0;
        cutCost   = 0.0;
        W0        = 0.0;
        W1        = 0.0;
        imbalance = 0.0;

        clevel = 0;
        cn     = 0;
        for (Int k = 0; k < n; k++)
        {
            externalDegree[k] = 0;
            bhIndex[k]        = 0;
            matching[k]       = 0;
        }
        singleton = -1;

        clearMarkArray();
    }

    Int *Gp    = p;
    double *Gx = x;
    double *Gw = w;

    /* Compute worst-case gains, and compute X. */
    double *gains = vertexGains;
    double min    = fabs((Gx) ? Gx[0] : 1);
    double max    = fabs((Gx) ? Gx[0] : 1);
    for (Int k = 0; k < n; k++)
    {
        W += (Gw) ? Gw[k] : 1;
        double sumEdgeWeights = 0.0;

        for (Int j = Gp[k]; j < Gp[k + 1]; j++)
        {
            double Gxj = (Gx) ? Gx[j] : 1;
            sumEdgeWeights += Gxj;

            if (fabs(Gxj) < min)
            {
                min = fabs(Gxj);
            }
            if (fabs(Gxj) > max)
            {
                max = fabs(Gxj);
            }
        }

        gains[k] = -sumEdgeWeights;
        X += sumEdgeWeights;
    }
    H = 2.0 * X;

    // May need to correct tolerance for very ill-conditioned problems
    worstCaseRatio = max / (1E-9 + min);

    initialized = true;
}

void EdgeCutProblem::clearMarkArray()
{
    markValue += 1;
    if (markValue < 0)
    {
        resetMarkArray();
    }
}

void EdgeCutProblem::clearMarkArray(Int incrementBy)
{
    markValue += incrementBy;
    if (markValue < 0)
    {
        resetMarkArray();
    }
}

void EdgeCutProblem::resetMarkArray()
{
    markValue = 1;
    for (Int k = 0; k < n; k++)
    {
        markArray[k] = 0;
    }
}

} // end namespace Mongoose
