#ifndef CONDITIONING_HPP_
#define CONDITIONING_HPP_

#include "mongoose_internal.hpp"

namespace SuiteSparse_Mongoose
{

Graph *conditionGraph
(
    Graph *G,
    Options *O,
    bool resetEW = false,
    bool resetNW = false
);

}

#endif
