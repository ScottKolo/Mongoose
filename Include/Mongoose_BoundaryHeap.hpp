#pragma once

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"

/* Mongoose BoundaryHeap-related Macros */
#ifndef MONGOOSE_BOUNDARYHEAP_MACROS
#define MONGOOSE_BOUNDARYHEAP_MACROS

  #define MONGOOSE_HEAP_PARENT(a)    (((a)-1) / 2)
  #define MONGOOSE_LEFT_CHILD(a)     (2*(a) + 1)
  #define MONGOOSE_RIGHT_CHILD(a)    (2*(a) + 2)

  #define MONGOOSE_IN_BOUNDARY(v)     (bhIndex[(v)] > 0)
  #define MONGOOSE_PUT_BHINDEX(v,p)    bhIndex[(v)] = (p) + 1;
  #define MONGOOSE_GET_BHINDEX(v)     (bhIndex[(v)]-1)

#endif

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
    Int *bhIndex,
    Int *bhHeap,
    double *gains,
    Int vertex,
    Int position,
    double gain
);

void heapifyDown
(
    Int *bhIndex,
    Int *bhHeap,
    Int size,
    double *gains,
    Int vertex,
    Int position,
    double gain
);

} // end namespace Mongoose
