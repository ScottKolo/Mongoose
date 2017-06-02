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
int ComputeEdgeSeparator(Graph *graph)
{
    // use default options if not present
    Options *options = Options::Create ( ) ;
    if (!options) return (EXIT_FAILURE) ;
    int result = ComputeEdgeSeparator(graph, options) ;
    SuiteSparse_free (options) ;
    return (result) ;
}

int ComputeEdgeSeparator(Graph *graph, Options *options)
{
    // Check inputs
    if (!optionsAreValid(options)) return (EXIT_FAILURE) ;
    std::srand(options->randomSeed);

    if (!graph) return EXIT_FAILURE;

    /* Finish initialization */
    if (!graph->initialize(options)) return EXIT_FAILURE;

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

    return EXIT_SUCCESS;
}

bool optionsAreValid(Options *options)
{
    if (!options)
    {
        // Error!
        return (false) ;
    }
    if (options->targetSplit < 0 || options->targetSplit > 1)
    {
        // Error!
        return (false) ;
    }
    if (options->softSplitTolerance < 0)
    {
        // Error!
        return (false) ;
    }
    return (true) ;
}

} // end namespace Mongoose
