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
int ComputeEdgeSeparator(Graph *G)
{
    // use default options if not present
    Options *O = Options::Create ( ) ;
    if (!O) return (EXIT_FAILURE) ;
    int result = ComputeEdgeSeparator(G, O) ;
    SuiteSparse_free (O) ;
    return (result) ;
}

int ComputeEdgeSeparator(Graph *G, Options *O)
{
    // Check inputs
    if (!optionsAreValid(O)) return (EXIT_FAILURE) ;
    std::srand(O->randomSeed);

    if (!G) return EXIT_FAILURE;

    /* Finish initialization */
    if (!G->initialize(O)) return EXIT_FAILURE;

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
    if (options->tolerance < 0)
    {
        // Error!
        return (false) ;
    }
    return (true) ;
}

} // end namespace Mongoose
