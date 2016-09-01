
#include "Mongoose_Waterdance.hpp"
#include "Mongoose_ImproveFM.hpp"
#include "Mongoose_ImproveQP.hpp"

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