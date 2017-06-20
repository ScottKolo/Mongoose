#pragma once

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

void bhLoad(Graph*, Options*);
void bhClear(Graph*);
void bhInsert(Graph*, Int vertex);

void bhRemove
(
    Graph*,
    Options*,
    Int vertex,
    double gain,
    bool partition,
    Int bhPosition
);

void heapifyUp
(
    Graph*,
    Int *bhHeap,
    double *gains,
    Int vertex,
    Int position,
    double gain
);

void heapifyDown
(
    Graph*,
    Int *bhHeap,
    Int size,
    double *gains,
    Int vertex,
    Int position,
    double gain
);

} // end namespace Mongoose
