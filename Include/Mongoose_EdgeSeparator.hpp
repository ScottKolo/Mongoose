#ifndef Mongoose_EdgeSeparator_hpp
#define Mongoose_EdgeSeparator_hpp

#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

int ComputeEdgeSeparator(Graph *G, Options *O);
bool initialize(Graph *G, Options *O);

} // end namespace Mongoose

#endif
