/* ========================================================================== */
/* === Include/Mongoose_BoundaryHeap.hpp ==================================== */
/* ========================================================================== */

/* -----------------------------------------------------------------------------
 * Mongoose Graph Partitioning Library  Copyright (C) 2017-2018,
 * Scott P. Kolodziej, Nuri S. Yeralan, Timothy A. Davis, William W. Hager
 * Mongoose is licensed under Version 3 of the GNU General Public License.
 * Mongoose is also available under other licenses; contact authors for details.
 * -------------------------------------------------------------------------- */

#pragma once

#include "Mongoose_Graph.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

void bhLoad(Graph *, const Options *);
void bhClear(Graph *);
void bhInsert(Graph *, Int vertex);

void bhRemove(Graph *, const Options *, Int vertex, double gain, bool partition,
              Int bhPosition);

void heapifyUp(Graph *, Int *bhHeap, double *gains, Int vertex, Int position,
               double gain);

void heapifyDown(Graph *, Int *bhHeap, Int size, double *gains, Int vertex,
                 Int position, double gain);

} // end namespace Mongoose
