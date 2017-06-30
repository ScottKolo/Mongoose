#include "Mongoose_Internal.hpp"
#include "Mongoose_GuessCut.hpp"
#include "Mongoose_ImproveQP.hpp"
#include "Mongoose_Waterdance.hpp"

namespace Mongoose
{

//-----------------------------------------------------------------------------
// This function takes a graph with options and computes the initial guess cut
//-----------------------------------------------------------------------------
bool guessCut(Graph *graph, Options *options)
{
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

    /* Do the waterdance refinement. */
    waterdance(graph, options);

    return true;
}

} // end namespace Mongoose
