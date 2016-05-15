#ifndef CONDITIONING_HPP_
#define CONDITIONING_HPP_

#include "mongoose_internal.hpp"

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
