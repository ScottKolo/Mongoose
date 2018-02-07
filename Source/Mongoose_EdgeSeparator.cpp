#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_Coarsening.hpp"
#include "Mongoose_GuessCut.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_Random.hpp"
#include "Mongoose_Refinement.hpp"
#include "Mongoose_Waterdance.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

bool optionsAreValid(const Options *options);
void cleanup(Graph *graph);

int ComputeEdgeSeparator(Graph *graph)
{
    // use default options if not present
    Options *options = Options::Create();

    if (!options)
        return (EXIT_FAILURE);

    int result = ComputeEdgeSeparator(graph, options);

    options->~Options();

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
    graph->initialize(options);

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

    cleanup(current);

    return EXIT_SUCCESS;
}

bool optionsAreValid(const Options *options)
{
    if (!options)
    {
        LogError("Fatal Error: options struct cannot be NULL.");
        return (false);
    }

    if (options->coarsenLimit < 1)
    {
        LogError("Fatal Error: options->coarsenLimit cannot be less than one.");
        return (false);
    }

    if (options->davisBrotherlyThreshold < 0)
    {
        LogError("Fatal Error: options->davisBrotherlyThreshold cannot be less than zero.");
        return (false);
    }

    if (options->numDances < 0)
    {
        LogError("Fatal Error: options->numDances cannot be less than zero.");
        return (false);
    }

    if (options->fmSearchDepth < 0)
    {
        LogError("Fatal Error: options->fmSearchDepth cannot be less than zero.");
        return (false);
    }

    if (options->fmConsiderCount < 0)
    {
        LogError("Fatal Error: options->fmConsiderCount cannot be less than zero.");
        return (false);
    }

    if (options->fmMaxNumRefinements < 0)
    {
        LogError("Fatal Error: options->fmMaxNumRefinements cannot be less than zero.");
        return (false);
    }

    if (options->gradProjTolerance < 0)
    {
        LogError("Fatal Error: options->gradProjTolerance cannot be less than zero.");
        return (false);
    }

    if (options->gradprojIterationLimit < 0)
    {
        LogError("Fatal Error: options->gradProjIterationLimit cannot be less than zero.");
        return (false);
    }

    if (options->targetSplit < 0 || options->targetSplit > 1)
    {
        LogError("Fatal Error: options->targetSplit must be in the range [0, 1].");
        return (false);
    }

    if (options->softSplitTolerance < 0)
    {
        LogError("Fatal Error: options->softSplitTolerance cannot be less than zero.");
        return (false);
    }

    return (true);
}

void cleanup(Graph *G)
{
    Int cutSize  = 0;
    double assoc = 0;

    for (Int i = 0; i < G->n; i++)
    {
        if (G->partition[i])
        {
            cutSize += G->externalDegree[i];

            for (Int j = G->p[i]; j < G->p[i + 1]; j++)
            {
                assoc += (G->x) ? G->x[j] : 1;
            }
        }
    }

    G->cutSize = cutSize;
    G->normCut = G->cutCost/assoc + G->cutCost/(G->X - assoc);
}

} // end namespace Mongoose
