/* ========================================================================== */
/* === Source/Mongoose_Waterdance.cpp ======================================= */
/* ========================================================================== */

/* -----------------------------------------------------------------------------
 * Mongoose Graph Partitioning Library  Copyright (C) 2017-2018,
 * Scott P. Kolodziej, Nuri S. Yeralan, Timothy A. Davis, William W. Hager
 * Mongoose is licensed under Version 3 of the GNU General Public License.
 * Mongoose is also available under other licenses; contact authors for details.
 * -------------------------------------------------------------------------- */

#include "Mongoose_Waterdance.hpp"
#include "Mongoose_ImproveFM.hpp"
#include "Mongoose_ImproveQP.hpp"
#include "Mongoose_Internal.hpp"

namespace Mongoose
{

void waterdance(Graph *graph, const Options *options)
{
    Int numDances = options->numDances;
    for (Int i = 0; i < numDances; i++)
    {
        improveCutUsingFM(graph, options);
        improveCutUsingQP(graph, options);
    }
}

} // end namespace Mongoose
