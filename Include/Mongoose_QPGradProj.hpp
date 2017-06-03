#pragma once

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_QPDelta.hpp"

namespace Mongoose
{

double qpGradProj(Graph*, Options*, QPDelta*);

} // end namespace Mongoose

