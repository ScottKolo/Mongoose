#include "Mongoose_Graph.hpp"

#include <algorithm>
#include <new>

namespace Mongoose
{

/* Constructor & Destructor */
Graph::Graph()
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

Graph *Graph::Create(const Int _n, const Int _nz, const bool allocate)
{
    void *memoryLocation = SuiteSparse_malloc(1, sizeof(Graph));
    if (!memoryLocation)
        return NULL;

    // Placement new
    Graph *graph = new (memoryLocation) Graph();

    size_t n = static_cast<size_t>(_n);
    graph->n = _n;

    size_t nz = static_cast<size_t>(_nz);
    graph->nz = _nz;

    graph->p     = (allocate) ? (Int *)SuiteSparse_malloc(n + 1, sizeof(Int)) : NULL;
    graph->i     = (allocate) ? (Int *)SuiteSparse_malloc(nz, sizeof(Int)) : NULL;
    graph->x     = (allocate) ? (double *)SuiteSparse_malloc(nz, sizeof(double)) : NULL;
    graph->w     = (allocate) ? (double *)SuiteSparse_malloc(n, sizeof(double)) : NULL;
    graph->X     = 0.0;
    graph->W     = 0.0;
    graph->H     = 0.0;
    if (allocate && (!graph->p || !graph->i || !graph->x || !graph->w))
    {
        graph->~Graph();
        return NULL;
    }

    graph->partition      = (bool *)SuiteSparse_malloc(n, sizeof(bool));
    graph->vertexGains    = (double *)SuiteSparse_malloc(n, sizeof(double));
    graph->externalDegree = (Int *)SuiteSparse_calloc(n, sizeof(Int));
    graph->bhIndex        = (Int *)SuiteSparse_calloc(n, sizeof(Int));
    graph->bhHeap[0]      = (Int *)SuiteSparse_malloc(n, sizeof(Int));
    graph->bhHeap[1]      = (Int *)SuiteSparse_malloc(n, sizeof(Int));
    graph->bhSize[0] = graph->bhSize[1] = 0;
    if (!graph->partition || !graph->vertexGains || !graph->externalDegree
        || !graph->bhIndex || !graph->bhHeap[0] || !graph->bhHeap[1])
    {
        graph->~Graph();
        return NULL;
    }

    graph->heuCost   = 0.0;
    graph->cutCost   = 0.0;
    graph->W0        = 0.0;
    graph->W1        = 0.0;
    graph->imbalance = 0.0;

    graph->parent      = NULL;
    graph->clevel      = 0;
    graph->cn          = 0;
    graph->matching    = (Int *)SuiteSparse_calloc(n, sizeof(Int));
    graph->matchmap    = (Int *)SuiteSparse_malloc(n, sizeof(Int));
    graph->invmatchmap = (Int *)SuiteSparse_malloc(n, sizeof(Int));
    graph->matchtype   = (Int *)SuiteSparse_malloc(n, sizeof(Int));
    graph->markArray   = (Int *)SuiteSparse_calloc(n, sizeof(Int));
    graph->markValue   = 1;
    graph->singleton   = -1;
    if (!graph->matching || !graph->matchmap || !graph->invmatchmap || !graph->markArray
        || !graph->matchtype)
    {
        graph->~Graph();
        return NULL;
    }

    graph->initialized = false;

    return graph;
}

Graph *Graph::Create(Graph *_parent)
{
    Graph *graph = Create(_parent->cn, _parent->nz, true);
    if (!graph)
        return NULL;

    graph->W           = _parent->W;
    graph->parent      = _parent;
    graph->clevel      = graph->parent->clevel + 1;

    return graph;
}

Graph *Graph::Create(cs *matrix)
{
    Graph *graph = Create(std::max(matrix->n, matrix->m), matrix->p[matrix->n], false);
    if(!graph)
    {
        return NULL;
    }

    graph->p = matrix->p;
    graph->i = matrix->i;
    graph->x = matrix->x;
    graph->w = (double *)SuiteSparse_malloc(static_cast<size_t>(graph->n),
                                                         sizeof(double));

    /* If we failed to attach weights, free the graph and return. */
    if (!graph->w)
    {
        /* Undo the brain transplant, free the Graph skeleton, and return. */
        graph->p = NULL;
        graph->i = NULL;
        graph->x = NULL;
        graph->~Graph();
        return NULL;
    }

    // Initialize vertex weights
    for (Int k = 0; k < graph->n; k++)
        graph->w[k] = 1.0;

    return graph;
}

Graph::~Graph()
{
    p = (Int *)SuiteSparse_free(p);
    i = (Int *)SuiteSparse_free(i);
    x = (double *)SuiteSparse_free(x);
    w = (double *)SuiteSparse_free(w);

    partition      = (bool *)SuiteSparse_free(partition);
    vertexGains    = (double *)SuiteSparse_free(vertexGains);
    externalDegree = (Int *)SuiteSparse_free(externalDegree);
    bhIndex        = (Int *)SuiteSparse_free(bhIndex);
    bhHeap[0]      = (Int *)SuiteSparse_free(bhHeap[0]);
    bhHeap[1]      = (Int *)SuiteSparse_free(bhHeap[1]);
    matching    = (Int *)SuiteSparse_free(matching);
    matchmap    = (Int *)SuiteSparse_free(matchmap);
    invmatchmap = (Int *)SuiteSparse_free(invmatchmap);
    matchtype   = (Int *)SuiteSparse_free(matchtype);

    markArray = (Int *)SuiteSparse_free(markArray);

    SuiteSparse_free(this);
}

/* Initialize a top level graph with a a set of options. */
void Graph::initialize(const Options *options)
{
    (void)options; // Unused variable

    if (initialized)
    {
        // Graph has been previously initialized. We need to clear some extra
        // data structures to be able to reuse it.

        X     = 0.0;
        W     = 0.0;
        H     = 0.0;

        bhSize[0] = bhSize[1] = 0;

        heuCost   = 0.0;
        cutCost   = 0.0;
        W0        = 0.0;
        W1        = 0.0;
        imbalance = 0.0;

        clevel      = 0;
        cn          = 0;
        for (Int k = 0; k < n; k++)
        {
            externalDegree[k] = 0;
            bhIndex[k]        = 0;
            matching[k]       = 0;
        }
        singleton   = -1;

        clearMarkArray();
    }

    Int *Gp    = p;
    double *Gx = x;
    double *Gw = w;

    /* Compute worst-case gains, and compute X. */
    double *gains = vertexGains;
    double min    = fabs( (Gx) ? Gx[0] : 1 );
    double max    = fabs( (Gx) ? Gx[0] : 1 );
    for (Int k = 0; k < n; k++)
    {
        W += Gw[k];
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

void Graph::clearMarkArray()
{
    markValue += 1;
    if (markValue < 0)
    {
        resetMarkArray();
    }
}

void Graph::clearMarkArray(Int incrementBy)
{
    markValue += incrementBy;
    if (markValue < 0)
    {
        resetMarkArray();
    }
}

void Graph::mark(Int index) { markArray[index] = markValue; }

void Graph::unmark(Int index) { markArray[index] = 0; }

bool Graph::isMarked(Int index) { return markArray[index] == markValue; }

Int Graph::getMarkValue() { return markValue; }

void Graph::resetMarkArray()
{
    markValue = 1;
    for (Int k = 0; k < n; k++)
    {
        markArray[k] = 0;
    }
}

} // end namespace Mongoose
