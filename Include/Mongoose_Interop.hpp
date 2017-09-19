#pragma once

#include "Mongoose_CSparse.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_Options.hpp"
#include "string.h"

namespace Mongoose
{

/* Configure an on-stack CSparse matrix from an existing Mongoose Graph. */
cs *GraphToCSparse3(Graph *, bool copy = false);
Graph *CSparse3ToGraph(cs *, bool resetEW = false, bool resetNW = false);

} // end namespace Mongoose
