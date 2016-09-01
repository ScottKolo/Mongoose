#ifndef Mongoose_ImproveFM_hpp
#define Mongoose_ImproveFM_hpp

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_CutCost.hpp"

namespace Mongoose
{

/* Swap candidates have the following features: */
struct SwapCandidate
{
    Int vertex;
    bool partition;
    Weight nodeWeight;
    Weight gain;
    Weight heuCost;
    Int bhPosition;
    Weight imbalance;

    SwapCandidate(){
        vertex = 0;
        partition = false;
        nodeWeight = 0.0;
        gain = -INFINITY;
        heuCost = INFINITY;
        bhPosition = -1;
        imbalance = 0.0;
    }
};

void improveCutUsingFM
(
    Graph *G,
    Options *O
);

void fmSwap
(
    Graph *G,
    Options *O,
    Int vertex,
    Weight gain,
    bool oldPartition,
    Int *mark,
    Int markValue
);

void calculateGain
(
    Graph *G,
    Options *O,
    Int vertex,
    Weight *out_gain,
    Int *out_externalDegree
);

} // end namespace Mongoose

#endif
