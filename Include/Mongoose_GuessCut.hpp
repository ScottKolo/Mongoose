#pragma once

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_CutCost.hpp"
#include "Mongoose_BoundaryHeap.hpp"

namespace Mongoose
{

bool guessCut(Graph*, Options*);

void pseudoperipheralGuess(Graph*, Options*);
bool findAllPseudoperipheralNodes(Graph*, Options*, Int *list, Int *listsize);
Int diagBFS(Graph*, Options*, Int *stack, Int *start);
void partBFS(Graph*, Options*, Int start);

} // end namespace Mongoose

