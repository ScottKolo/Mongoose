#pragma once

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

void match(Graph*, const Options*);

void matching_Random(Graph*, const Options*);
void matching_HEM(Graph*, const Options*);
void matching_PA(Graph*, const Options*);
void matching_DavisPA(Graph*, const Options*);
void matching_LabelProp(Graph*, const Options*);
void matching_Cleanup(Graph*, const Options*);

} // end namespace Mongoose
