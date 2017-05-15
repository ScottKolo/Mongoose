// TODO : make a single user include file
#ifndef Mongoose_EdgeSeparator_hpp
#define Mongoose_EdgeSeparator_hpp

#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

int ComputeEdgeSeparator(Graph *G) ;
int ComputeEdgeSeparator(Graph *G, Options *O);
bool initialize(Graph *graph, Options *options);

} // end namespace Mongoose

#endif
