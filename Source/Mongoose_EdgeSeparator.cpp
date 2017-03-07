#include "Mongoose_Internal.hpp"
#include "Mongoose_Coarsening.hpp"
#include "Mongoose_GuessCut.hpp"
#include "Mongoose_Refinement.hpp"
#include "Mongoose_Waterdance.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

bool optionsAreValid(Options *options);

/* The input must be a single connected component. */
int ComputeEdgeSeparator(Graph *G, Options *O)
{
    /* Check inputs */
    if (!G || !O) return EXIT_FAILURE;

    /* Finish initialization */
    if (!initialize(G, O)) return EXIT_FAILURE;

    /* Keep track of what the current graph is at any stage */
    Graph *current = G;

    /* If we need to coarsen the graph, do the coarsening. */
    while (current->n >= O->coarsenLimit)
    {
        match(current, O);
        Graph *next = coarsen(current, O);

        /* If we ran out of memory during coarsening, unwind the stack. */
        if (!next)
        {
            while (current != G)
            {
                next = current->parent;
                current->~Graph();
                SuiteSparse_free(current);
                current = next;
            }
            return EXIT_FAILURE;
        }

        current = next;
    }

    /*
     * Generate a guess cut and do FM refinement.
     * On failure, unwind the stack.
     */
    if (!guessCut(current, O))
    {
        while (current != G)
        {
            Graph *next = current->parent;
            current->~Graph();
            SuiteSparse_free(current);
            current = next;
        }
        return EXIT_FAILURE;
    }

    /*
     * Refine the guess cut back to the beginning.
     */
    while (current->parent != NULL)
    {
        current = refine(current, O);
        waterdance(current, O);
    }

    return EXIT_SUCCESS;
}

/* Finish the initialization of the top level graph. */
bool initialize(Graph *graph, Options *options)
{
    if (!optionsAreValid(options))
    {
        return false;
    }
    Int n = graph->n;
    Int *Gp = graph->p;
    Weight *Gx = graph->x;
    Weight *Gw = graph->w;

    graph->cn = 0;
    graph->matching =       (Int*) SuiteSparse_calloc(n, sizeof(Int));
    graph->matchmap =       (Int*) SuiteSparse_calloc(n, sizeof(Int));
    graph->invmatchmap =    (Int*) SuiteSparse_malloc(n, sizeof(Int));
    graph->matchtype =      (Int*) SuiteSparse_calloc(n, sizeof(Int));
    graph->mark =           (Int*) SuiteSparse_calloc(n, sizeof(Int));
    graph->markValue = 1;

    graph->partition =     (bool*) SuiteSparse_malloc(n, sizeof(bool));
    graph->bhIndex =        (Int*) SuiteSparse_calloc(n, sizeof(Int));
    graph->bhHeap[0] =      (Int*) SuiteSparse_malloc(n, sizeof(Int));
    graph->bhHeap[1] =      (Int*) SuiteSparse_malloc(n, sizeof(Int));
    graph->vertexGains = (Weight*) SuiteSparse_malloc(graph->n, sizeof(Weight));
    graph->externalDegree = (Int*) SuiteSparse_calloc(n, sizeof(Int));

    /* Check memory and abort if necessary. */
    if (!graph->matching || !graph->matchmap || !graph->invmatchmap || !graph->matchtype ||
        !graph->mark || !graph->partition || !graph->bhIndex || !graph->bhHeap[0] ||
        !graph->bhHeap[1] ||
        !graph->vertexGains || !graph->externalDegree)
    {
        graph->matching =       (Int*) SuiteSparse_free(graph->matching);
        graph->matchmap =       (Int*) SuiteSparse_free(graph->matchmap);
        graph->invmatchmap =    (Int*) SuiteSparse_free(graph->invmatchmap);
        graph->matchtype =      (Int*) SuiteSparse_free(graph->matchtype);
        graph->mark =           (Int*) SuiteSparse_free(graph->mark);
        graph->partition =     (bool*) SuiteSparse_free(graph->partition);
        graph->bhIndex =        (Int*) SuiteSparse_free(graph->bhIndex);
        graph->bhHeap[0] =      (Int*) SuiteSparse_free(graph->bhHeap[0]);
        graph->bhHeap[1] =      (Int*) SuiteSparse_free(graph->bhHeap[1]);
        graph->vertexGains = (Weight*) SuiteSparse_free(graph->vertexGains);
        graph->externalDegree = (Int*) SuiteSparse_free(graph->externalDegree);
        return false;
    }

    /* Compute worst-case gains, and compute X. */
    Weight X = 0.0, W = 0.0;
    Weight *gains = graph->vertexGains;
    for (Int k = 0; k < n; k++)
    {
        W += Gw[k];
        Weight sumEdgeWeights = 0.0;

        for (Int p = Gp[k]; p < Gp[k+1]; p++) sumEdgeWeights += Gx[p];

        gains[k] = -sumEdgeWeights;
        X += sumEdgeWeights;
    }
    graph->X = X;
    graph->W = W;
    graph->H = 2.0 * X;
    return true;
}

bool optionsAreValid(Options *options)
{
    if (options->targetSplit < 0 || options->targetSplit > 1)
    {
        // Error!
    }

    return true;
}

} // end namespace Mongoose
