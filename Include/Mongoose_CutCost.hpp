#pragma once

namespace Mongoose
{

/* A partitioning has the following metrics: */
struct CutCost
{
    double heuCost;   /* Sum of cutCost and balance penalty     */
    double cutCost;   /* Sum of edge weights in the cut set.    */
    double W[2];      /* Sum of node weights in each partition. */
    double imbalance; /* targetSplit - (W[0] / W)               */
};

} // end namespace Mongoose
