/* ========================================================================== */
/* === Include/Mongoose_Matching.hpp ======================================== */
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

void match(Graph *, const Options *);

void matching_Random(Graph *, const Options *);
void matching_HEM(Graph *, const Options *);
void matching_SR(Graph *, const Options *);
void matching_SRdeg(Graph *, const Options *);
void matching_Cleanup(Graph *, const Options *);

} // end namespace Mongoose
