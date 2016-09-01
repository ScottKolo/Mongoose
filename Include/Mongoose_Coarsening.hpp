#ifndef Mongoose_Coarsening_hpp
#define Mongoose_Coarsening_hpp

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_Matching.hpp"

namespace Mongoose
{

Graph *coarsen(Graph *G, Options *O);

} // end namespace Mongoose

#endif
