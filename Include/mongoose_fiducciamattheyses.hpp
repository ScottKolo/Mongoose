#ifndef FIDUCCIAMATTHEYSES_HPP_
#define FIDUCCIAMATTHEYSES_HPP_

#include "mongoose_internal.hpp"
#include "mongoose_cutcost.hpp"

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

void fmRefine
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

#endif /* FIDUCCIAMATTHEYSES_HPP_ */
