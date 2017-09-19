#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_Coarsening.hpp"
#include "Mongoose_GuessCut.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_Random.hpp"
#include "Mongoose_Refinement.hpp"
#include "Mongoose_Waterdance.hpp"

#include <algorithm>

namespace Mongoose
{

bool optionsAreValid(const Options *options);
void cleanup(Graph *graph, const Options *options);

/* The input must be a single connected component. */
int ComputeEdgeSeparator(Graph *graph)
{
    // use default options if not present
    Options *options = Options::Create();
    if (!options)
        return (EXIT_FAILURE);
    int result = ComputeEdgeSeparator(graph, options);
    SuiteSparse_free(options);
    return (result);
}

int ComputeEdgeSeparator(Graph *graph, const Options *options)
{
    // Check inputs
    if (!optionsAreValid(options))
        return (EXIT_FAILURE);

    setRandomSeed(options->randomSeed);

    if (!graph)
        return EXIT_FAILURE;

    /* Finish initialization */
    if (!graph->initialize(options))
        return EXIT_FAILURE;

    /* Keep track of what the current graph is at any stage */
    Graph *current = graph;

    /* If we need to coarsen the graph, do the coarsening. */
    while (current->n >= options->coarsenLimit)
    {
        match(current, options);
        Graph *next = coarsen(current, options);

        /* If we ran out of memory during coarsening, unwind the stack. */
        if (!next)
        {
            while (current != graph)
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
    if (!guessCut(current, options))
    {
        while (current != graph)
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
        current = refine(current, options);
        waterdance(current, options);
    }

    cleanup(current, options);

    return EXIT_SUCCESS;
}

bool optionsAreValid(const Options *options)
{
    if (!options)
    {
        // Error!
        return (false);
    }
    if (options->targetSplit < 0 || options->targetSplit > 1)
    {
        // Error!
        return (false);
    }
    if (options->softSplitTolerance < 0)
    {
        // Error!
        return (false);
    }
    return (true);
}

void cleanup(Graph *G, const Options *options)
{
    Int cutSize        = 0;
    double cutCost     = 0;
    double part_weight = 0;
    for (Int i = 0; i < G->n; i++)
    {
        if (G->partition[i])
        {
            part_weight += (G->w) ? G->w[i] : 1;
            for (Int j = G->p[i]; j < G->p[i + 1]; j++)
            {
                if (i != j && (!G->partition[G->i[j]]))
                {
                    cutSize += 1;
                    cutCost += (G->x) ? G->x[j] : 1;
                }
            }
        }
    }
    G->imbalance = options->targetSplit
                   - std::min(part_weight, G->W - part_weight) / G->W;
    G->cutCost = cutCost;
    G->cutSize = cutSize;
}

} // end namespace Mongoose
