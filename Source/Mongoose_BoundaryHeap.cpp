#include "Mongoose_Internal.hpp"
#include "Mongoose_BoundaryHeap.hpp"
#include "Mongoose_CutCost.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

//-----------------------------------------------------------------------------
// This function inserts the specified vertex into the graph
//-----------------------------------------------------------------------------
void bhLoad
(
    Graph *graph,
    Options *options
)
{
    /* Load the boundary heaps. */
    Int n = graph->n;
    Int *Gp = graph->p;
    Int *Gi = graph->i;
    double *Gx = graph->x;
    double *Gw = graph->w;
    bool *partition = graph->partition;
    double *gains = graph->vertexGains;
    Int *externalDegree = graph->externalDegree;

    /* Keep track of the cut cost. */
    CutCost cost;
    cost.heuCost = 0.0;
    cost.cutCost = 0.0;
    cost.W[0] = 0.0;
    cost.W[1] = 0.0;
    cost.imbalance = 0.0;

    /* Compute the gains & discover if the vertex is on the boundary. */
    for (Int k = 0; k < n; k++)
    {
        bool kPartition = partition[k];
        cost.W[kPartition] += Gw[k];

        double gain = 0.0;
        Int exD = 0;
        for (Int p = Gp[k]; p < Gp[k+1]; p++)
        {
            double edgeWeight = Gx[p];
            bool onSameSide = (kPartition == partition[Gi[p]]);
            gain += (onSameSide ? -edgeWeight : edgeWeight);
            if (!onSameSide)
            {
                exD++;
                cost.cutCost += edgeWeight;
            }
        }
        gains[k] = gain;
        externalDegree[k] = exD;
        if (exD > 0) bhInsert(graph, k);
    }

    /* Save the cut cost to the graph. */
    graph->cutCost = cost.cutCost;
    graph->W0 = cost.W[0];
    graph->W1 = cost.W[1];

    double targetSplit = options->targetSplit ;
    if (targetSplit > 0.5) targetSplit = 1. - targetSplit ;

    graph->imbalance = targetSplit - std::min(graph->W0, graph->W1) / graph->W;
    graph->heuCost = (graph->cutCost + (fabs(graph->imbalance) > options->softSplitTolerance
                                ? fabs(graph->imbalance) * graph->H
                                : 0.0));
}

//-----------------------------------------------------------------------------
// This function inserts the specified vertex into the graph's boundary heap
//-----------------------------------------------------------------------------
void bhInsert
(
    Graph *graph,
    Int vertex
)
{
    /* Unpack structures */
    Int vp = graph->partition[vertex];
    Int *bhIndex = graph->bhIndex;
    Int *bhHeap = graph->bhHeap[vp];
    Int size = graph->bhSize[vp];
    double *gains = graph->vertexGains;

    bhHeap[size] = vertex;
    MONGOOSE_PUT_BHINDEX(vertex, size);

    heapifyUp(bhIndex, bhHeap, gains, vertex, size, gains[vertex]);

    /* Save the size. */
    graph->bhSize[vp] = size+1;
}

//-----------------------------------------------------------------------------
// This function clears the entire boundary heap
//-----------------------------------------------------------------------------
void bhClear
(
    Graph *graph
)
{
    /* Clear the index entries for the heaps. */
    Int *bhIndex = graph->bhIndex;
    Int *externalDegree = graph->externalDegree;
    for (Int h = 0; h < 2; h++)
    {
        Int *bhHeap = graph->bhHeap[h];
        Int size = graph->bhSize[h];
        for (Int i = 0; i < size; i++)
        {
            Int v = bhHeap[i];
            bhIndex[v] = 0;
            externalDegree[v] = 0;
        }
    }

    /* Clear the size. */
    graph->bhSize[0] = graph->bhSize[1] = 0;
}


//-----------------------------------------------------------------------------
// Removes the specified vertex from its heap.
// To do this, we swap the last element in the heap with the element we
// want to remove. Then we heapify up and heapify down.
//-----------------------------------------------------------------------------
void bhRemove
(
    Graph *graph,
    Options *options,
    Int vertex,
    double gain,
    bool partition,
    Int bhPosition
)
{
    (void)options; // Unused variable
    (void)gain;    // Unused variable

    double *gains = graph->vertexGains;
    Int *bhIndex = graph->bhIndex;
    Int *bhHeap = graph->bhHeap[partition];
    Int size = (--graph->bhSize[partition]);

    /* If we removed the last position in the heap, there's nothing to do. */
    if (bhPosition == size) { bhIndex[vertex] = 0; return; }

    /* Replace the vertex with the last element in the heap. */
    Int v = bhHeap[bhPosition] = bhHeap[size];
    MONGOOSE_PUT_BHINDEX(v, bhPosition);

    /* Finish the delete of "vertex" from the heap. */
    bhIndex[vertex] = 0;

    /* Bubble up then bubble down with a reread of v because it may have
     * changed during heapifyUp. */
    heapifyUp(bhIndex, bhHeap, gains, v, bhPosition, gains[v]);
    v = bhHeap[bhPosition];
    heapifyDown(bhIndex, bhHeap, size, gains, v, bhPosition, gains[v]);
}

//-----------------------------------------------------------------------------
// Starting at a position, this function will heapify from a vertex upwards
//-----------------------------------------------------------------------------
void heapifyUp
(
    Int *bhIndex,
    Int *bhHeap,
    double *gains,
    Int vertex,
    Int position,
    double gain
)
{
    if (position == 0) return;

    Int posParent = MONGOOSE_HEAP_PARENT(position);
    Int pVertex = bhHeap[posParent];
    double pGain = gains[pVertex];

    /* If we need to swap this node with the parent then: */
    if (pGain < gain)
    {
        bhHeap[posParent] = vertex;
        bhHeap[position] = pVertex;
        MONGOOSE_PUT_BHINDEX(vertex, posParent);
        MONGOOSE_PUT_BHINDEX(pVertex, position);
        heapifyUp(bhIndex, bhHeap, gains, vertex, posParent, gain);
    }
}

//-----------------------------------------------------------------------------
// Starting at a position, this function will heapify from a vertex downwards
//-----------------------------------------------------------------------------
void heapifyDown
(
    Int *bhIndex,
    Int *bhHeap,
    Int size,
    double *gains,
    Int vertex,
    Int position,
    double gain
)
{
    if (position >= size) return;

    Int lp = MONGOOSE_LEFT_CHILD(position);
    Int rp = MONGOOSE_RIGHT_CHILD(position);

    Int lv = (lp < size ? bhHeap[lp] : -1);
    Int rv = (rp < size ? bhHeap[rp] : -1);

    double lg = (lv >= 0 ? gains[lv] : -INFINITY);
    double rg = (rv >= 0 ? gains[rv] : -INFINITY);

    if (gain < lg || gain < rg)
    {
        if (lg > rg)
        {
            bhHeap[position] = lv;
            MONGOOSE_PUT_BHINDEX(lv, position);
            bhHeap[lp] = vertex;
            MONGOOSE_PUT_BHINDEX(vertex, lp);
            heapifyDown(bhIndex, bhHeap, size, gains, vertex, lp, gain);
        }
        else
        {
            bhHeap[position] = rv;
            MONGOOSE_PUT_BHINDEX(rv, position);
            bhHeap[rp] = vertex;
            MONGOOSE_PUT_BHINDEX(vertex, rp);
            heapifyDown(bhIndex, bhHeap, size, gains, vertex, rp, gain);
        }
    }
}

} // end namespace Mongoose
