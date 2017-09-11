// TODO : make a single user include file
#pragma once

#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

int ComputeEdgeSeparator(Graph*) ;
int ComputeEdgeSeparator(Graph*, const Options*);

} // end namespace Mongoose

