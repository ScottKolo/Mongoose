/**
 * @file Mongoose_Coarsening.hpp
 * @author Nuri Yeralan, Scott Kolodziej
 * @brief Coarsening of a graph given a previously determined matching
 *
 * @details In order to operate on extremely large graphs, a pre-processing is 
 * done to reduce the size of the graph while maintaining its overall structure.
 * Given a matching of vertices with other vertices (e.g. heavy edge matching, 
 * random, etc.), coarsening constructs the new, coarsened graph.
 */

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
