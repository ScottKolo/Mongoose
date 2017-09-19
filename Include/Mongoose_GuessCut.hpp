#pragma once

#include "Mongoose_BoundaryHeap.hpp"
#include "Mongoose_CutCost.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

bool guessCut(Graph *, const Options *);

} // end namespace Mongoose
