#ifndef MONGOOSE_CUTCOST_HPP_
#define MONGOOSE_CUTCOST_HPP_

namespace Mongoose
{

/* A partitioning has the following metrics: */
struct CutCost
{
    Weight heuCost;     /* Sum of cutCost and balance penalty     */
    Weight cutCost;     /* Sum of edge weights in the cut set.    */
    Weight W[2];        /* Sum of node weights in each partition. */
    Weight imbalance;   /* targetSplit - (W[0] / W)               */
};

} // end namespace Mongoose

#endif /* CUTCOST_HPP_ */
