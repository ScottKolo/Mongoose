/**
 * Graph data structure.
 *
 * Stores graph adjacency and weight information. Also used as a container for
 * storing information about matching, coarsening, and partitioning.
 */

#ifndef Mongoose_Graph_hpp
#define Mongoose_Graph_hpp

#include "stdio.h"
#include "Mongoose_Internal.hpp"

namespace Mongoose
{

class Graph
{
public:
    /** CSparse3 Interoperability ********************************************/
    Int cs_n;                            /** # columns                       */
    Int cs_m;                            /** # rows                          */
    Int cs_nz;                           /** # triplet entries or -1         */
    Int cs_nzmax;                        /** max # nonzeros                  */

    /** Graph Data ***********************************************************/
    Int n;                               /** # vertices                      */
    Int nz;                              /** # edges                         */
    Int *p;                              /** Column pointers                 */
    Int *i;                              /** Row indices                     */
    Weight *x;                           /** Edge weight                     */
    Weight *w;                           /** Node weight                     */
    Weight X;                            /** Sum of edge weights             */
    Weight W;                            /** Sum of node weights             */

    Weight H;                            /** Heuristic max penalty to assess */

    /** Partition Data *******************************************************/
    bool *partition;                     /** T/F denoting partition side     */
    Weight *vertexGains;                 /** Gains for each vertex           */
    Int *externalDegree;                 /** # edges lying across the cut    */
    Int *bhIndex;                        /** Index+1 of a vertex in the heap */
    Int *bhHeap[2];                      /** Heap data structure organized by
                                            boundaryGains descending         */
    Int bhSize[2];                       /** Size of the boundary heap       */

    /** Cut Cost Metrics *****************************************************/
    Weight heuCost;                      /** cutCost + balance penalty       */
    Weight cutCost;                      /** Sum of edge weights in cut set  */
    Weight W0;                           /** Sum of partition 0 node weights */
    Weight W1;                           /** Sum of partition 1 node weights */
    Weight imbalance;                    /** Degree to which the partitioning
                                             is imbalanced, and this is
                                             computed as (0.5 - W0/W).       */

    /** Matching Data ********************************************************/
    Graph *parent;                       /** Link to the parent graph        */
    Int clevel;                          /** Coarsening level for this graph */
    Int cn;                              /** # vertices in coarse graph      */
    Int *matching;                       /** Linked List of matched vertices */
    Int *matchmap;                       /** Map from fine to coarse vertices */
    Int *invmatchmap;                    /** Map from coarse to fine vertices */
    Int *matchtype;                      /** Vertex's match classification
                                              0: Orphan
                                              1: Standard (random, hem, shem)
                                              2: Brotherly
                                              3: Community                   */

    /** Mark Data ************************************************************/
    Int *mark;                           /** O(n) mark array                 */
    Int markValue;                       /** Mark array can be cleared in O(k)
                                             by incrementing markValue.
                                             Implicitly, a mark value less than
                                             markValue is unmarked.          */

    /* Constructor & Destructor */
    Graph();
    static Graph *Create (const Int _n, const Int _nz);
    static Graph *Create (Graph *_parent);
    ~Graph();
};

} // end namespace Mongoose

/* Mongoose graph-related macros */
#ifndef MONGOOSE_MARKED
#define MONGOOSE_MARKED(a)   (mark[(a)] == markValue)
#endif

#ifndef MONGOOSE_MARK
#define MONGOOSE_MARK(a)     (mark[(a)] = markValue);
#endif

#endif
