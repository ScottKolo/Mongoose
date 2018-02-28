/* ========================================================================== */
/* === Include/Mongoose_ImproveQP.hpp ======================================= */
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
#include "Mongoose_QPDelta.hpp"
#include "Mongoose_QPGradProj.hpp"

namespace Mongoose
{

bool improveCutUsingQP(Graph *, const Options *, bool isInitial = false);

} // end namespace Mongoose
