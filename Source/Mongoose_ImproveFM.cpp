
#include "Mongoose_ImproveFM.hpp"
#include "Mongoose_QPBoundaryHeap.hpp"
#include "Mongoose_Interop.hpp"

// TODO why doesn't this code use O->targetSplit ???
// TODO how is tolerance define?  if target balance is .5 and
// tolerance is 0.1, is the allowed range [0.4,0.6]?  Or [0.45,0.55]?

namespace Mongoose
{

void fmRefine_worker(Graph *G, Options *O);

//-----------------------------------------------------------------------------
// Wrapper for Fidducia-Mattheyes cut improvement.
//-----------------------------------------------------------------------------
void improveCutUsingFM(Graph *G, Options *O)
{
    Logger::tic(FMTiming);

    if (!O->useFM) return;

    Weight heuCost = INFINITY;
    for (Int i = 0; i < O->fmMaxNumRefinements && G->heuCost < heuCost; i++)
    {
        heuCost = G->heuCost;
        fmRefine_worker(G, O);
    }

    Logger::toc(FMTiming);
}

//-----------------------------------------------------------------------------
// Make a number of partition moves while considering the impact on problem balance.
//-----------------------------------------------------------------------------
void fmRefine_worker(Graph *G, Options *O)
{
    Weight *Gw = G->w;
    Weight W = G->W;
    Int **bhHeap  = G->bhHeap;
    Int *bhIndex = G->bhIndex;
    Int *bhSize = G->bhSize;
    Int *externalDegree = G->externalDegree;
    Weight *gains = G->vertexGains;
    bool *partition = G->partition;

    Int *mark = G->mark;
    Int markValue = G->markValue;

    /* Keep a stack of moved nodes. */
    Int *stack = G->matchmap;
    Int head = 0, tail = 0;

    /* Create & initialize a working cost and a best cost. */
    struct CutCost workingCost, bestCost;
    workingCost.heuCost   = bestCost.heuCost   = G->heuCost;
    workingCost.cutCost   = bestCost.cutCost   = G->cutCost;
    workingCost.W[0]      = bestCost.W[0]      = G->W0;
    workingCost.W[1]      = bestCost.W[1]      = G->W1;
    workingCost.imbalance = bestCost.imbalance = G->imbalance;

    /* Tolerance and the linear penalty to assess. */
    Weight tol = O->tolerance;
    Weight H = G->H;

    // fix tolerance if out of range
    if (tol < 0)
    {
        O->tolerance = tol = 0. ;
    }
    printf ("FM tol %g\n", tol) ;

    Int fmSearchDepth = O->fmSearchDepth;
    Int fmConsiderCount = O->fmConsiderCount;
    Int i = 0;
    bool productive = true;
    for (; i < fmSearchDepth && productive; i++)
    {
        productive = false;

        /* Look for the best vertex to swap: */
        struct SwapCandidate bestCandidate;
        for (Int h = 0; h < 2; h++)
        {
            Int *heap = bhHeap[h];
            Int size = bhSize[h];
            for (Int c = 0; c < fmConsiderCount && c < size; c++)
            {
                /* Read the vertex, and if it's marked, try the next one. */
                Int v = heap[c];
                if (MONGOOSE_MARKED(v)) continue;

                /* Read the gain for the vertex. */
                Weight gain = gains[v];

                /* The balance penalty is the penalty to assess for the move. */
                Weight nodeWeight = Gw[v];
                Weight imbalance = workingCost.imbalance + (h ? -1.0 : 1.0) *
                                   (nodeWeight / W);
                Weight absImbalance = fabs(imbalance);
                Weight imbalanceDelta = absImbalance - fabs(workingCost.imbalance);

                /* If the move hurts the balance past tol, add a penalty. */
                Weight balPenalty = 0.0;
                if (imbalanceDelta > 0 && absImbalance > tol)
                {
                    balPenalty = absImbalance * H;
                }

                /* Heuristic cost is the cut cost reduced by the gain for making this move.
                 * The gain for the move is amplified by any impact to the balance penalty. */
                Weight heuCost = workingCost.cutCost - (gain - balPenalty);

                /* If our heuristic value is better than the running one: */
                if (heuCost < bestCandidate.heuCost)
                {
                    bestCandidate.vertex = v;
                    bestCandidate.partition = h;
                    bestCandidate.nodeWeight = nodeWeight;
                    bestCandidate.gain = gain;
                    bestCandidate.bhPosition = c;
                    bestCandidate.imbalance = imbalance;
                    bestCandidate.heuCost = heuCost;
                }
            }
        }

        /* If we were able to find the best unmoved boundary vertex: */
        if (bestCandidate.heuCost < INFINITY)
        {
            productive = true;
            MONGOOSE_MARK(bestCandidate.vertex);

            /* Move the vertex from the boundary into the move set. */
            bhRemove(G, O, bestCandidate.vertex, bestCandidate.gain,
                     bestCandidate.partition, bestCandidate.bhPosition);
            stack[tail++] = bestCandidate.vertex;

            /* Swap & update the vertex and its neighbors afterwards. */
            fmSwap
            (
                G, O,
                bestCandidate.vertex,
                bestCandidate.gain,
                bestCandidate.partition,
                mark, markValue
            );

            /* Update the cut cost. */
            workingCost.cutCost -= 2.0*bestCandidate.gain; /* x2 because of symmetry */
            workingCost.W[bestCandidate.partition]  -= bestCandidate.nodeWeight;
            workingCost.W[!bestCandidate.partition] += bestCandidate.nodeWeight;
            workingCost.imbalance = bestCandidate.imbalance;
            Weight absImbalance = fabs(bestCandidate.imbalance);
            workingCost.heuCost = workingCost.cutCost +
                                  (absImbalance > tol ? absImbalance * H : 0.0);

            /* Commit the cut if it's better. */
            if (workingCost.heuCost < bestCost.heuCost)
            {
                bestCost = workingCost;
                head = tail;
                i = 0;
            }
        }
    }

    /* We've exhausted our search space, so undo all suboptimal moves. */
    for (Int u = tail-1; u >= head; u--)
    {
        Int vertex = stack[u];
        Int bhVertexPosition = MONGOOSE_GET_BHINDEX(vertex);

        /* Unmark this vertex. */
        mark[vertex] = markValue-1;     // TODO better to set to zero

        /* It is possible, although rare, that a vertex may have gone
         * from not in the boundary to an undo state that places it in
         * the boundary. It is also possible that a previous swap added
         * this vertex to the boundary already. */
        if (bhVertexPosition != -1)
        {
            bhRemove(G, O, vertex, gains[vertex], partition[vertex],
                     bhVertexPosition);
        }

        /* Swap the partition and compute the impact on neighbors. */
        fmSwap
        (
            G, O,
            vertex,
            gains[vertex],
            partition[vertex],
            mark, markValue
        );

        if (externalDegree[vertex] > 0) bhInsert(G, vertex);
    }

    // clear the marks from all the nodes
    MONGOOSE_CLEAR_ALL_MARKS ;      // TODO: reset if int overflow

    /* Re-add any vertices that were moved that are still on the boundary. */
    for (Int i = 0; i < head; i++)
    {
        Int vertex = stack[i];
        if (externalDegree[vertex] > 0 && !MONGOOSE_IN_BOUNDARY(vertex))
        {
            bhInsert(G, vertex);
        }
    }

    // clear the marks from all the nodes
    MONGOOSE_CLEAR_ALL_MARKS ;      // TODO: reset if int overflow
    G->markValue = markValue ;

    /* Save the best cost back into the graph. */
    G->heuCost   = bestCost.heuCost;
    G->cutCost   = bestCost.cutCost;
    G->W0        = bestCost.W[0];
    G->W1        = bestCost.W[1];
    G->imbalance = bestCost.imbalance;
}

//-----------------------------------------------------------------------------
// This function swaps the partition of a vertex
//-----------------------------------------------------------------------------
void fmSwap
(
    Graph *G,
    Options *O,
    Int vertex,
    Weight gain,
    bool oldPartition,
    Int *mark,
    Int markValue
)
{
    Int *Gp = G->p;
    Int *Gi = G->i;
    Weight *Gx = G->x;
    bool *partition = G->partition;
    Weight *gains = G->vertexGains;
    Int *externalDegree = G->externalDegree;
    Int *bhIndex = G->bhIndex;
    Int **bhHeap = G->bhHeap;
    Int *bhSize = G->bhSize;

    /* Swap partitions */
    bool newPartition = !oldPartition;
    partition[vertex] = newPartition;
    gains[vertex] = -gain;

    /* Update neighbors. */
    Int exD = 0;
    for (Int p = Gp[vertex]; p < Gp[vertex+1]; p++)
    {
        Int neighbor = Gi[p];
        bool neighborPartition = partition[neighbor];
        bool sameSide = (newPartition == neighborPartition);

        /* Update the bestCandidate vertex's external degree. */
        if (!sameSide) exD++;

        /* Update the neighbor's gain. */
        Weight edgeWeight = Gx[p];
        Weight neighborGain = gains[neighbor];
        neighborGain += 2 * (sameSide ? -edgeWeight : edgeWeight);
        gains[neighbor] = neighborGain;

        /* Update the neighbor's external degree. */
        Int neighborExD = externalDegree[neighbor];
        neighborExD += (sameSide ? -1 : 1);
        externalDegree[neighbor] = neighborExD;

        Int position = MONGOOSE_GET_BHINDEX(neighbor);

        /* If the neighbor was in a heap: */
        if (position != -1)
        {
            /* If it had its externalDegree reduced to 0, remove it from the heap. */
            if (neighborExD == 0)
            {
                bhRemove
                (
                    G, O,
                    neighbor,
                    neighborGain,
                    neighborPartition,
                    position
                );
            }
            /* If the neighbor is in the heap, we touched its gain
             * so make sure the heap property is satisfied. */
            else
            {
                Int v = neighbor;
                heapifyUp(bhIndex, bhHeap[neighborPartition], gains, v,
                          position, neighborGain);
                v = bhHeap[neighborPartition][position];
                heapifyDown(bhIndex, bhHeap[neighborPartition],
                            bhSize[neighborPartition], gains, v, position,
                            gains[v]);
            }
        }
        /* Else the neighbor wasn't in the heap so add it. */
        else
        {
            if (!MONGOOSE_MARKED(neighbor))
            {
                assert(!MONGOOSE_IN_BOUNDARY(neighbor));
                bhInsert(G, neighbor);
            }
        }
    }
    externalDegree[vertex] = exD;
}

//-----------------------------------------------------------------------------
// This function computes the gain of a vertex
//-----------------------------------------------------------------------------
void calculateGain
(
    Graph *G,
    Options *O,
    Int vertex,
    Weight *out_gain,
    Int *out_externalDegree
)
{
    Int *Gp = G->p;
    Int *Gi = G->i;
    Weight *Gx = G->x;
    bool *partition = G->partition;

    bool vp = partition[vertex];

    Weight gain = 0.0;
    Int externalDegree = 0;
    for (Int p = Gp[vertex]; p < Gp[vertex+1]; p++)
    {
        Weight ew = (Gx ? Gx[p] : 1.0);
        bool sameSide = (partition[Gi[p]] == vp);
        gain += (sameSide ? -ew : ew);

        if (!sameSide) externalDegree++;
    }

    /* Save outputs */
    *out_gain = gain;
    *out_externalDegree = externalDegree;
}

} // end namespace Mongoose
