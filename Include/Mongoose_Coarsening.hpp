/**
 * Coarsening of a graph given a previously determined matching
 *
 * In order to operate on extremely large graphs, a pre-processing is
 * done to reduce the size of the graph while maintaining its overall structure.
 * Given a matching of vertices with other vertices (e.g. heavy edge matching, 
 * random, etc.), coarsening constructs the new, coarsened graph.
 */

#pragma once

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_Matching.hpp"

namespace Mongoose
{

Graph *coarsen(Graph*, const Options*);

} // end namespace Mongoose

