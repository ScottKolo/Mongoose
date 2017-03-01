#include "Mongoose_Internal.hpp"
#include "Mongoose_ImproveQP.hpp"
#include "Mongoose_BoundaryHeap.hpp"
#include "Mongoose_ImproveFM.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

void improveCutUsingQP
(
    Graph *G,
    Options *O,
    bool isInitial
)
{
    if (!O->useQPGradProj) return;

    Logger::tic(QPTiming);

    /* Unpack structure fields */
    Int n = G->n;
    Int *Gp = G->p;
    Weight *Gx = G->x;
    Weight *Gw = G->w;
    Weight *gains = G->vertexGains;
    Int *externalDegree = G->externalDegree;
    Int *bhIndex = G->bhIndex;

    /* Create workspaces */
    QPDelta *QP = QPDelta::Create(n);
    if (!QP)
    {
        Logger::toc(QPTiming);
        return;
    }

    Weight *D = QP->D;

    /* Convert the guess from discrete to continuous. */
    Double *guess = QP->x;
    bool *partition = G->partition;
    for (Int k = 0; k < n; k++)
    {
        if (isInitial)
        {
            guess[k] = 0.5;
        }
        else
        {
            if (partition[k])
            {
                guess[k] = MONGOOSE_IN_BOUNDARY(k) ? 0.75 : 1.0;
            }
            else
            {
                guess[k] = MONGOOSE_IN_BOUNDARY(k) ? 0.25 : 0.0;
            }
        }

        Weight maxWeight = -INFINITY;
        for (Int p = Gp[k]; p < Gp[k+1]; p++)
        {
            maxWeight = MONGOOSE_MAX2(maxWeight, Gx[p]);
        }
        D[k] = maxWeight;
    }

    // set the QP parameters, and fix if out of range
    Double tol = O->tolerance ;
    Double targetSplit = O->targetSplit ;
    if (tol < 0)
    {
        // ensure tolerance is positive
        O->tolerance = tol = 0. ;
    }
    if (targetSplit <= 0 || targetSplit >= 1)
    {
        // targetSplit is out range.  default is a 50/50 cut
        O->targetSplit = targetSplit = 0.5 ;
    }
    if (targetSplit > 0.5)
    {
        // TODO this is wrong ... !
        // targetSplit can be 0.7 for example, and this is not the
        // same as 0.3.
        // ensure targetSplit is in the range 0 to 0.5, but only to
        // compute lo and hi.  Do not update O->targetSplit.
        targetSplit = 1. - targetSplit ;
    }
    QP->lo = G->W * (targetSplit - tol) ;
    QP->hi = G->W * ((1. - targetSplit) + tol) ;
    ASSERT (QP->lo <= QP->hi) ;

    // Build the FreeSet, compute grad, possibly adjust QP->lo and QP->hi
    QPlinks(G, O, QP);

    /* Do one run of gradient projection. */
    QPgradproj(G, O, QP);
    QPboundary(G, O, QP);
    QPgradproj(G, O, QP);
    QPboundary(G, O, QP);

    /* Use the CutCost to keep track of impacts to the cut cost. */
    CutCost cost;
    cost.cutCost = G->cutCost;
    cost.W[0] = G->W0;
    cost.W[1] = G->W1;
    cost.imbalance = G->imbalance;

    /* Do the recommended swaps and compute the new cut cost. */
    Int *mark = G->mark;
    Int markValue = G->markValue;

    for (Int k = 0; k < n; k++)
    {
        bool newPartition = (guess[k] > 0.5);
        bool oldPartition = partition[k];

        if (newPartition != oldPartition)
        {
            /* Update the cut cost. */
            cost.cutCost -= 2 * gains[k];
            cost.W[oldPartition] -= Gw[k];
            cost.W[newPartition] += Gw[k];
            cost.imbalance = O->targetSplit - cost.W[0] / G->W;

            Int bhVertexPosition = MONGOOSE_GET_BHINDEX(k);

            /* It is possible, although rare, that a vertex may have gone
             * from not in the boundary to an undo state that places it in
             * the boundary. It is also possible that a previous swap added
             * this vertex to the boundary already. */
            if (bhVertexPosition != -1)
            {
                bhRemove(G, O, k, gains[k], partition[k], bhVertexPosition);
            }

            /* Swap the partition and compute the impact on neighbors. */
            fmSwap
            (
                G, O,
                k,
                gains[k],
                partition[k],
                mark, markValue
            );

            if (externalDegree[k] > 0) bhInsert(G, k);
        }
    }

    // clear the marks from all the nodes
    MONGOOSE_CLEAR_ALL_MARKS ;      // TODO: reset if int overflow
    G->markValue = markValue ;

    /* Free the QP structure */
    QP->~QPDelta();
    SuiteSparse_free(QP);

    /* Write the cut cost back to the graph. */
    G->cutCost = cost.cutCost;
    G->W0 = cost.W[0];
    G->W1 = cost.W[1];
    G->imbalance = cost.imbalance;
    Weight absImbalance = fabs(G->imbalance);
    G->heuCost = G->cutCost +
                 (absImbalance > O->tolerance ? absImbalance * G->H : 0.0);

    Logger::toc(QPTiming);
}

} // end namespace Mongoose
