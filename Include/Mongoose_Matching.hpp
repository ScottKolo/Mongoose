#pragma once

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

void match(Graph*, Options*);

void matching_Random(Graph*, Options*);
void matching_HEM(Graph*, Options*);
void matching_PA(Graph*, Options*);
void matching_DavisPA(Graph*, Options*);
void matching_LabelProp(Graph*, Options*);
void matching_Cleanup(Graph*, Options*);

} // end namespace Mongoose
