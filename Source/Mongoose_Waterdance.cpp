#include "Mongoose_Internal.hpp"
#include "Mongoose_Waterdance.hpp"
#include "Mongoose_ImproveFM.hpp"
#include "Mongoose_ImproveQP.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

void waterdance(Graph *G, Options *O)
{
    Int numDances = O->numDances;
    for (Int i = 0; i < numDances; i++)
    {
        improveCutUsingFM(G, O);
        improveCutUsingQP(G, O);
    }
}

} // end namespace Mongoose
