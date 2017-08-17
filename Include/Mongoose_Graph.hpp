/**
 * Graph data structure.
 *
 * Stores graph adjacency and weight information. Also used as a container for
 * storing information about matching, coarsening, and partitioning.
 */

#pragma once

#include "Mongoose_Internal.hpp"
#include "Mongoose_Options.hpp"

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
    double *x;                           /** Edge weight                     */
    double *w;                           /** Node weight                     */
    double X;                            /** Sum of edge weights             */
    double W;                            /** Sum of node weights             */

    double H;                            /** Heuristic max penalty to assess */
    double worstCaseRatio;

    /** Partition Data *******************************************************/
    bool *partition;                     /** T/F denoting partition side     */
    double *vertexGains;                 /** Gains for each vertex           */
    Int *externalDegree;                 /** # edges lying across the cut    */
    Int *bhIndex;                        /** Index+1 of a vertex in the heap */
    Int *bhHeap[2];                      /** Heap data structure organized by
                                            boundaryGains descending         */
    Int bhSize[2];                       /** Size of the boundary heap       */

    /** Cut Cost Metrics *****************************************************/
    double heuCost;                      /** cutCost + balance penalty       */
    double cutCost;                      /** Sum of edge weights in cut set  */
    Int cutSize;                         /** Number of edges in cut set      */
    double W0;                           /** Sum of partition 0 node weights */
    double W1;                           /** Sum of partition 1 node weights */
    double imbalance;                    /** Degree to which the partitioning
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
    Int singleton;



    /* Constructor & Destructor */
    Graph();
    static Graph *Create (Int _n, Int _nz);
    static Graph *Create (Graph *_parent);
    ~Graph();
    bool initialize(Options *options);

    /** Matching Functions ****************************************************/
    inline bool isMatched(Int vertex)
    {
        return (matching[vertex] > 0);
    }

    inline Int getMatch(Int vertex)
    {
        return (matching[vertex]-1);
    }

    inline void createMatch(Int vertexA, Int vertexB, MatchType matchType)
    {
        matching[vertexA] = (vertexB)+1;
        matching[vertexB] = (vertexA)+1;
        invmatchmap[cn] = vertexA;
        matchtype[vertexA] = matchType;
        matchtype[vertexB] = matchType;
        matchmap[vertexA] = cn;
        matchmap[vertexB] = cn;
        cn++;
    }

    inline void createCommunityMatch(Int vertexA, Int vertexB, MatchType matchType)
    {
        Int vm[4] = {-1,-1,-1,-1};
        vm[0] = vertexA;
        vm[1] = getMatch(vm[0]);
        vm[2] = getMatch(vm[1]);
        vm[3] = getMatch(vm[2]);

        bool is3Way = (vm[0] == vm[3]);
        if(is3Way)
        {
            matching[vm[1]] = vertexA+1;
            createMatch(vm[2], vertexB, matchType);
        }
        else
        {
            matching[vertexB] = matching[vertexA];
            matching[vertexA] = vertexB+1;
            matchmap[vertexB] = matchmap[vertexA];
            matchtype[vertexB] = matchType;
        }
    }

    /** Boundary Heap Functions ***********************************************/
    inline Int BH_getParent(Int a)
    {
        return ((a - 1) / 2);
    }

    inline Int BH_getLeftChild(Int a)
    {
        return (2*a + 1);
    }

    inline Int BH_getRightChild(Int a)
    {
        return (2*a + 2);
    }

    inline bool BH_inBoundary(Int v)
    {
        return (bhIndex[v] > 0);
    }

    inline void BH_putIndex(Int v, Int pos)
    {
        bhIndex[v] = (pos + 1);
    }

    inline Int BH_getIndex(Int v)
    {
        return (bhIndex[v] - 1);
    }

    /** Mark Array Functions **************************************************/
    void clearMarkArray();
    void clearMarkArray(Int incrementBy);
    void mark(Int index);
    void unmark(Int index);
    Int getMarkValue();
    bool isMarked(Int index);

private:
    /** Mark Data ************************************************************/
    Int *markArray;                      /** O(n) mark array                 */
    Int markValue;                       /** Mark array can be cleared in O(k)
                                             by incrementing markValue.
                                             Implicitly, a mark value less than
                                             markValue is unmarked.          */
    void resetMarkArray();
};

} // end namespace Mongoose

