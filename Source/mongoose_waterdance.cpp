
#include "mongoose_waterdance.hpp"

namespace Mongoose
{

void waterdance(Graph *G, Options *O)
{
    Int numDances = O->numDances;
    for (Int i = 0; i < numDances; i++)
    {
        fmRefine(G, O);
        qpGradProj(G, O);
    }
}

} // end namespace Mongoose