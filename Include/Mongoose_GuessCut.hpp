#pragma once

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_CutCost.hpp"
#include "Mongoose_BoundaryHeap.hpp"

namespace Mongoose
{

bool guessCut(Graph *graph, Options *options);

void pseudoperipheralGuess(Graph *graph, Options *options);
bool findAllPseudoperipheralNodes(Graph *graph, Options *options, Int *list, Int *listsize);

Int diagBFS
(
    Graph *graph,
    Options *options,
    Int *stack,
    Int *start
);

void partBFS
(
    Graph *graph,
    Options *options,
    Int start
);

} // end namespace Mongoose

