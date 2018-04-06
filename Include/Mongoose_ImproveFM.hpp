/* ========================================================================== */
/* === Include/Mongoose_ImproveFM.hpp ======================================= */
/* ========================================================================== */

/* -----------------------------------------------------------------------------
 * Mongoose Graph Partitioning Library  Copyright (C) 2017-2018,
 * Scott P. Kolodziej, Nuri S. Yeralan, Timothy A. Davis, William W. Hager
 * Mongoose is licensed under Version 3 of the GNU General Public License.
 * Mongoose is also available under other licenses; contact authors for details.
 * -------------------------------------------------------------------------- */

#pragma once

#include "Mongoose_CutCost.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_Options.hpp"

namespace Mongoose
{

/* Swap candidates have the following features: */
struct SwapCandidate
{
    Int vertex;
    bool partition;
    double vertexWeight;
    double gain;
    double heuCost;
    Int bhPosition;
    double imbalance;

    SwapCandidate()
    {
        vertex       = 0;
        partition    = false;
        vertexWeight = 0.0;
        gain         = -INFINITY;
        heuCost      = INFINITY;
        bhPosition   = -1;
        imbalance    = 0.0;
    }
};

void improveCutUsingFM(Graph *, const Options *);

void fmSwap(Graph *, const Options *, Int vertex, double gain,
            bool oldPartition);
void fmRefine_worker(Graph *graph, const Options *options);
void calculateGain(Graph *, const Options *, Int vertex, double *out_gain,
                   Int *out_externalDegree);

} // end namespace Mongoose
