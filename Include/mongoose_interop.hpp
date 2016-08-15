#ifndef MONGOOSE_INTEROP_HPP_
#define MONGOOSE_INTEROP_HPP_

#include "string.h"
#include "mongoose_cs.hpp"
#include "mongoose_internal.hpp"

namespace Mongoose
{

/* Configure an on-stack CSparse matrix from an existing Mongoose Graph. */
cs *GraphToCSparse3(Graph *G, bool copy = false);
Graph *CSparse3ToGraph(cs *G, bool resetEW = false, bool resetNW = false);

} // end namespace Mongoose

#endif
