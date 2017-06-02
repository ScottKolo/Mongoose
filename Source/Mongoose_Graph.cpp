#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

/* Constructor & Destructor */
Graph::Graph()
{
    cs_n = cs_m = cs_nz = cs_nzmax = 0;
    n = nz = 0;
    p = NULL;
    i = NULL;
    x = NULL;
    w = NULL;
    X = 0.0;
    W = 0.0;
    H = 0.0;

    partition = NULL;
    vertexGains = NULL;
    externalDegree = NULL;
    bhIndex = NULL;
    bhHeap[0] = bhHeap[1] = NULL;
    bhSize[0] = bhSize[1] = 0;

    heuCost = 0.0;
    cutCost = 0.0;
    W0 = 0.0;
    W1 = 0.0;
    imbalance = 0.0;

    parent = NULL;
    clevel = 0;
    cn = 0;
    matching = NULL;
    matchmap = NULL;
    invmatchmap = NULL;
    matchtype = NULL;

    markArray = NULL;
    markValue = 1;
}

Graph* Graph::Create (
    const Int _n,
    const Int _nz
)
{
    Graph *ret = static_cast<Graph*>(SuiteSparse_calloc(1, sizeof(Graph)));
    if(!ret) return NULL;

    size_t n = static_cast<size_t>(_n);
    ret->n = 
    ret->cs_n = 
    ret->cs_m = _n;

    size_t nz = static_cast<size_t>(_nz);
    ret->nz = 
    ret->cs_nzmax = _nz;

    ret->cs_nz = -1; /* Compressed Column Format */
    ret->p = (Int*) SuiteSparse_malloc(n+1, sizeof(Int));
    ret->i = (Int*) SuiteSparse_malloc(nz, sizeof(Int));
    ret->x = (double*) SuiteSparse_malloc(nz, sizeof(double));
    ret->w = (double*) SuiteSparse_malloc(n, sizeof(double));
    ret->X = 0.0;
    ret->W = 0.0;
    ret->H = 0.0;
    if(!ret->p || !ret->i || !ret->x || !ret->w)
    {
         ret->~Graph();
         SuiteSparse_free(ret);
         return NULL;
    }

    ret->partition = (bool*) SuiteSparse_malloc(n, sizeof(bool));
    ret->vertexGains = (double*) SuiteSparse_malloc(n, sizeof(double));
    ret->externalDegree = (Int*) SuiteSparse_calloc(n, sizeof(Int));
    ret->bhIndex = (Int*) SuiteSparse_calloc(n, sizeof(Int));
    ret->bhHeap[0] = (Int*) SuiteSparse_malloc(n, sizeof(Int));
    ret->bhHeap[1] = (Int*) SuiteSparse_malloc(n, sizeof(Int));
    ret->bhSize[0] = ret->bhSize[1] = 0;
    if(!ret->partition || !ret->vertexGains || !ret->externalDegree
    || !ret->bhIndex || !ret->bhHeap[0] || !ret->bhHeap[1])
    {
         ret->~Graph();
         SuiteSparse_free(ret);
         return NULL;
    }

    ret->heuCost = 0.0;
    ret->cutCost = 0.0;
    ret->W0 = 0.0;
    ret->W1 = 0.0;
    ret->imbalance = 0.0;

    ret->parent = NULL;
    ret->clevel = 0;
    ret->cn = 0;
    ret->matching = (Int*) SuiteSparse_calloc(n, sizeof(Int));
    ret->matchmap = (Int*) SuiteSparse_malloc(n, sizeof(Int));
    ret->invmatchmap = (Int*) SuiteSparse_malloc(n, sizeof(Int));
    ret->matchtype = (Int*) SuiteSparse_malloc(n, sizeof(Int));
    ret->markArray = (Int*) SuiteSparse_calloc(n, sizeof(Int));
    ret->markValue = 1;
    if(!ret->matching || !ret->matchmap || !ret->invmatchmap || !ret->markArray
    || !ret->matchtype )
    {
         ret->~Graph();
         SuiteSparse_free(ret);
         return NULL;
    }

    return ret;
}

Graph* Graph::Create (
    Graph *_parent
)
{
    Graph *ret = static_cast<Graph*>(SuiteSparse_calloc(1, sizeof(Graph)));
    if(!ret) return NULL;
    //new (ret) Graph();

    ret->n = 
    ret->cs_n = 
    ret->cs_m = _parent->cn;
    size_t n = static_cast<size_t>(ret->n);

    ret->nz = 
    ret->cs_nzmax = _parent->nz;
    size_t nz = static_cast<size_t>(ret->nz);

    ret->cs_nz = -1; /* Compressed Column Format */
    ret->p = (Int*) SuiteSparse_malloc((n+1), sizeof(Int));
    ret->i = (Int*) SuiteSparse_malloc(nz, sizeof(Int));
    ret->x = (double*) SuiteSparse_malloc(nz, sizeof(double));
    ret->w = (double*) SuiteSparse_malloc(n, sizeof(double));
    ret->X = 0.0;
    ret->W = _parent->W;
    ret->H = 0.0;
    if(!ret->p || !ret->i || !ret->x || !ret->w)
    {
         ret->~Graph();
         SuiteSparse_free(ret);
         return NULL;
    }

    ret->partition = (bool*) SuiteSparse_malloc(n, sizeof(bool));
    ret->vertexGains = (double*) SuiteSparse_malloc(n, sizeof(double));
    ret->externalDegree = (Int*) SuiteSparse_calloc(n, sizeof(Int));
    ret->bhIndex = (Int*) SuiteSparse_calloc(n, sizeof(Int));
    ret->bhHeap[0] = (Int*) SuiteSparse_malloc(n, sizeof(Int));
    ret->bhHeap[1] = (Int*) SuiteSparse_malloc(n, sizeof(Int));
    ret->bhSize[0] = ret->bhSize[1] = 0;
    if(!ret->partition || !ret->vertexGains || !ret->externalDegree
    || !ret->bhIndex || !ret->bhHeap[0] || !ret->bhHeap[1])
    {
         ret->~Graph();
         SuiteSparse_free(ret);
         return NULL;
    }

    ret->heuCost = 0.0;
    ret->cutCost = 0.0;
    ret->W0 = 0.0;
    ret->W1 = 0.0;
    ret->imbalance = 0.0;

    ret->parent = _parent;
    ret->clevel = ret->parent->clevel+1;
    ret->cn = 0;
    ret->matching = (Int*) SuiteSparse_calloc(n, sizeof(Int));
    ret->matchmap = (Int*) SuiteSparse_malloc(n, sizeof(Int));
    ret->invmatchmap = (Int*) SuiteSparse_malloc(n, sizeof(Int));
    ret->matchtype = (Int*) SuiteSparse_malloc(n, sizeof(Int));
    ret->markArray = (Int*) SuiteSparse_calloc(n, sizeof(Int));
    ret->markValue = 1;
    if(!ret->matching || !ret->matchmap || !ret->invmatchmap || !ret->markArray
    || !ret->matchtype )
    {
         ret->~Graph();
         SuiteSparse_free(ret);
         return NULL;
    }

    return ret;
}

Graph::~Graph()
{
    this->n  = cs_n  = cs_m     = 0;
    this->nz = cs_nz = cs_nzmax = 0;

    p = (Int*) SuiteSparse_free(p);
    i = (Int*) SuiteSparse_free(i);
    x = (double*) SuiteSparse_free(x);
    w = (double*) SuiteSparse_free(w);

    partition = (bool*) SuiteSparse_free(partition);
    vertexGains = (double*) SuiteSparse_free(vertexGains);
    externalDegree = (Int*) SuiteSparse_free(externalDegree);
    bhIndex = (Int*) SuiteSparse_free(bhIndex);
    bhHeap[0] = (Int*) SuiteSparse_free(bhHeap[0]);
    bhHeap[1] = (Int*) SuiteSparse_free(bhHeap[1]);

    cutCost = 0.0;
    W0 = 0.0;
    W1 = 0.0;
    imbalance = 0.0;

    clevel = 0;
    cn = 0;
    matching = (Int*) SuiteSparse_free(matching);
    matchmap = (Int*) SuiteSparse_free(matchmap);
    invmatchmap = (Int*) SuiteSparse_free(invmatchmap);
    matchtype = (Int*) SuiteSparse_free(matchtype);

    markArray = (Int*) SuiteSparse_free(markArray);
    markValue = 1;
}

/* Initialize a top level graph with a a set of options. */
bool Graph::initialize(Options *options)
{
    (void)options; // Unused variable

    Int *Gp = p;
    double *Gx = x;
    double *Gw = w;

    cn = 0;
    matching =       (Int*) SuiteSparse_calloc(n, sizeof(Int));
    matchmap =       (Int*) SuiteSparse_calloc(n, sizeof(Int));
    invmatchmap =    (Int*) SuiteSparse_malloc(n, sizeof(Int));
    matchtype =      (Int*) SuiteSparse_calloc(n, sizeof(Int));
    markArray =      (Int*) SuiteSparse_calloc(n, sizeof(Int));
    markValue = 1;

    partition =     (bool*) SuiteSparse_malloc(n, sizeof(bool));
    bhIndex =        (Int*) SuiteSparse_calloc(n, sizeof(Int));
    bhHeap[0] =      (Int*) SuiteSparse_malloc(n, sizeof(Int));
    bhHeap[1] =      (Int*) SuiteSparse_malloc(n, sizeof(Int));
    vertexGains = (double*) SuiteSparse_malloc(n, sizeof(double));
    externalDegree = (Int*) SuiteSparse_calloc(n, sizeof(Int));

    /* Check memory and abort if necessary. */
    if (!matching || !matchmap || !invmatchmap || !matchtype ||
        !markArray || !partition || !bhIndex || !bhHeap[0] ||
        !bhHeap[1] ||
        !vertexGains || !externalDegree)
    {
        matching =       (Int*) SuiteSparse_free(matching);
        matchmap =       (Int*) SuiteSparse_free(matchmap);
        invmatchmap =    (Int*) SuiteSparse_free(invmatchmap);
        matchtype =      (Int*) SuiteSparse_free(matchtype);
        markArray =      (Int*) SuiteSparse_free(markArray);
        partition =     (bool*) SuiteSparse_free(partition);
        bhIndex =        (Int*) SuiteSparse_free(bhIndex);
        bhHeap[0] =      (Int*) SuiteSparse_free(bhHeap[0]);
        bhHeap[1] =      (Int*) SuiteSparse_free(bhHeap[1]);
        vertexGains = (double*) SuiteSparse_free(vertexGains);
        externalDegree = (Int*) SuiteSparse_free(externalDegree);
        return false;
    }

    /* Compute worst-case gains, and compute X. */
    double *gains = vertexGains;
    for (Int k = 0; k < n; k++)
    {
        W += Gw[k];
        double sumEdgeWeights = 0.0;

        for (Int j = Gp[k]; j < Gp[k+1]; j++) sumEdgeWeights += Gx[j];

        gains[k] = -sumEdgeWeights;
        X += sumEdgeWeights;
    }
    H = 2.0 * X;
    return true;
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

void Graph::mark(Int index)
{
    markArray[index] = markValue;
}

// Only use this if you know what you're doing!
void Graph::mark(Int index, Int value)
{
    markArray[index] = value;
}

void Graph::unmark(Int index)
{
    markArray[index] = 0;
}

void Graph::checkForSpaceAndResetIfNeeded(Int incrementBy)
{
    if (markValue + incrementBy < markValue)
    {
        resetMarkArray();
    }
}

bool Graph::isMarked(Int index)
{
    return markArray[index] == markValue;
}

Int Graph::getMarkValue()
{
    return markValue;
}

Int Graph::getMarkArrayValue(Int index)
{
    return markArray[index];
}

void Graph::resetMarkArray()
{
    markValue = 1;
    for (Int k = 0; k < n; k++)
    {
        markArray[k] = 0;
    }
}

} // end namespace Mongoose
