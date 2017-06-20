#include "Mongoose_Internal.hpp"
#include "Mongoose_Waterdance.hpp"
#include "Mongoose_ImproveFM.hpp"
#include "Mongoose_ImproveQP.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

void waterdance(Graph *graph, Options *options)
{
    Int numDances = options->numDances;
    for (Int i = 0; i < numDances; i++)
    {
        improveCutUsingFM(graph, options);
        improveCutUsingQP(graph, options);
    }
}

} // end namespace Mongoose
