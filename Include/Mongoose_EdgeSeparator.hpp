// TODO : make a single user include file
#pragma once

#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

int ComputeEdgeSeparator(Graph *graph) ;
int ComputeEdgeSeparator(Graph *graph, Options *options);
bool initialize(Graph *graph, Options *options);

} // end namespace Mongoose

