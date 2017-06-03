#pragma once

#include "string.h"
#include "Mongoose_CSparse.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

/* Configure an on-stack CSparse matrix from an existing Mongoose Graph. */
cs *GraphToCSparse3(Graph *G, bool copy = false);
Graph *CSparse3ToGraph(cs *G, bool resetEW = false, bool resetNW = false);

} // end namespace Mongoose

