#ifndef Mongoose_Conditioning_hpp
#define Mongoose_Conditioning_hpp

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

Graph *conditionGraph
(
    Graph *G,
    Options *O,
    bool resetEW = false,
    bool resetNW = false
);

} // end namespace Mongoose

#endif
