#include "Mongoose_Internal.hpp"
#include "Mongoose_ImproveQP.hpp"
#include "Mongoose_BoundaryHeap.hpp"
#include "Mongoose_ImproveFM.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"
#include "Mongoose_QPNapsack.hpp"
#include "Mongoose_QPLinks.hpp"
#include "Mongoose_QPBoundary.hpp"

namespace Mongoose
{

bool improveCutUsingQP
(
    Graph *graph,
    Options *options,
    bool isInitial
)
{
    if (!options->useQPGradProj) return false;

    Logger::tic(QPTiming);

    /* Unpack structure fields */
    Int n = graph->n;
    Int *Gp = graph->p;
    double *Gx = graph->x;              // edge weights
    double *Gw = graph->w;              // node weights
    double *gains = graph->vertexGains;
    Int *externalDegree = graph->externalDegree;

    /* Create workspaces */
    QPDelta *QP = QPDelta::Create(n);
    if (!QP)
    {
        Logger::toc(QPTiming);
        return false;
    }

    // set the QP parameters
    double tol = options->softSplitTolerance;
    double targetSplit = options->targetSplit ;
    if (targetSplit > 0.5) targetSplit = 1. - targetSplit ;

    // ensure targetSplit and tolerance are valid.  These conditions were
    // already checked on input to Mongoose, in optionsAreValid.
    ASSERT (tol >= 0);
    ASSERT (targetSplit >= 0 && targetSplit <= 0.5);

    // QP upper and lower bounds.  targetSplit +/- tol is in the range 0 to 1,
    // and then this factor is multiplied by the sum of all node weights
    // (graph->W) to get the QP lo and hi.
    QP->lo = graph->W * std::max(0., targetSplit - tol);
    QP->hi = graph->W * std::min(1., targetSplit + tol);
    ASSERT (QP->lo <= QP->hi);

    /* Convert the guess from discrete to continuous. */
    double *D = QP->D;
    double *guess = QP->x;
    bool *partition = graph->partition;
    for (Int k = 0; k < n; k++)
    {
        if (isInitial)
        {
            guess[k] = targetSplit;
        }
        else
        {
            if (partition[k])
            {
                guess[k] = graph->BH_inBoundary(k) ? 0.75 : 1.0;
            }
            else
            {
                guess[k] = graph->BH_inBoundary(k) ? 0.25 : 0.0;
            }
        }
        double maxWeight = 0;
        for (Int p = Gp[k]; p < Gp[k+1]; p++)
        {
            maxWeight = std::max(maxWeight, Gx[p]);
        }
        D[k] = maxWeight;
    }

    // lo <= a'x <= hi might not hold here

    QP->lambda = 0;
    if (QP->b < QP->lo || QP->b > QP->hi)
    {
        QP->lambda = QPNapsack(guess, n, QP->lo, QP->hi, graph->w, QP->lambda,
                               QP->FreeSet_status,
                               QP->wx[1], QP->wi[0], QP->wi[1]);
    }

    // Build the FreeSet, compute grad, possibly adjust QP->lo and QP->hi
    if (!QPLinks(graph, options, QP))
    {
        Logger::toc(QPTiming);
        return false;
    }

    // lo <= a'x <= hi now holds (lo and hi are modified as needed in QPLinks)

    /* Do one run of gradient projection. */
    QPGradProj(graph, options, QP);
    QPBoundary(graph, options, QP);
    QPGradProj(graph, options, QP);
    QPBoundary(graph, options, QP);

    /* Use the CutCost to keep track of impacts to the cut cost. */
    CutCost cost;
    cost.cutCost = graph->cutCost;
    cost.W[0] = graph->W0;
    cost.W[1] = graph->W1;
    cost.imbalance = graph->imbalance;

    /* Do the recommended swaps and compute the new cut cost. */
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
            cost.imbalance = targetSplit -
                std::min(cost.W[0], cost.W[1]) / graph->W;

            Int bhVertexPosition = graph->BH_getIndex(k);

            /* It is possible, although rare, that a vertex may have gone
             * from not in the boundary to an undo state that places it in
             * the boundary. It is also possible that a previous swap added
             * this vertex to the boundary already. */
            if (bhVertexPosition != -1)
            {
                bhRemove(graph, options, k, gains[k], partition[k], bhVertexPosition);
            }

            /* Swap the partition and compute the impact on neighbors. */
            fmSwap
            (
                graph, options,
                k,
                gains[k],
                partition[k]
            );

            if (externalDegree[k] > 0) bhInsert(graph, k);
        }
    }

    // clear the marks from all the nodes
    graph->clearMarkArray();

    /* Free the QP structure */
    QP->~QPDelta();
    SuiteSparse_free(QP);

    /* Write the cut cost back to the graph. */
    graph->cutCost = cost.cutCost;
    graph->W0 = cost.W[0];
    graph->W1 = cost.W[1];
    graph->imbalance = cost.imbalance;
    double absImbalance = fabs(graph->imbalance);
    graph->heuCost = graph->cutCost +
                 (absImbalance > options->softSplitTolerance ? absImbalance * graph->H : 0.0);

    Logger::toc(QPTiming);

    return true;
}

} // end namespace Mongoose
