#include "Mongoose_Internal.hpp"
#include "Mongoose_GuessCut.hpp"
#include "Mongoose_ImproveQP.hpp"
#include "Mongoose_Random.hpp"
#include "Mongoose_Waterdance.hpp"

namespace Mongoose
{

//-----------------------------------------------------------------------------
// This function takes a graph with options and computes the initial guess cut
//-----------------------------------------------------------------------------
bool guessCut(Graph *graph, Options *options)
{
    switch (options->guessCutType)
    {
        case GuessQP:
            for (Int k = 0; k < graph->n; k++)
            {
                graph->partition[k] = false;
            }
            graph->partition[0] = true;

            bhLoad(graph, options);
            if (!improveCutUsingQP(graph, options, true))
            {
                return false;
                // Error - QP Failure
            }
            break;
        case GuessRandom:
            for (Int k = 0; k < graph->n; k++)
            {
                graph->partition[k] = (random() % 2 == 0) ? true : false;
            }

            bhLoad(graph, options);
            break;
        case GuessNaturalOrder:
            for (Int k = 0; k < graph->n; k++)
            {
                graph->partition[k] = (k < graph->n/2) ? true : false;
            }
            break;
    }

    /* Do the waterdance refinement. */
    waterdance(graph, options);

    return true;
}

} // end namespace Mongoose
