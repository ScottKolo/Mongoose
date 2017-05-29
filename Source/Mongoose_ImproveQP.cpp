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
    Graph *G,
    Options *O,
    bool isInitial
)
{
    if (!O->useQPGradProj) return false;

    Logger::tic(QPTiming);

    /* Unpack structure fields */
    Int n = G->n;
    Int *Gp = G->p;
    double *Gx = G->x;              // edge weights
    double *Gw = G->w;              // node weights
    double *gains = G->vertexGains;
    Int *externalDegree = G->externalDegree;
    Int *bhIndex = G->bhIndex;

    /* Create workspaces */
    QPDelta *QP = QPDelta::Create(n);
    if (!QP)
    {
        Logger::toc(QPTiming);
        return false;
    }

    // set the QP parameters
    double tol = O->tolerance;
    double targetSplit = O->targetSplit ;
    if (targetSplit > 0.5) targetSplit = 1. - targetSplit ;

    // ensure targetSplit and tolerance are valid.  These conditions were
    // already checked on input to Mongoose, in optionsAreValid.
    ASSERT (tol >= 0);
    ASSERT (targetSplit >= 0 && targetSplit <= 0.5);

    // QP upper and lower bounds.  targetSplit +/- tol is in the range 0 to 1,
    // and then this factor is multiplied by the sum of all node weights (G->W)
    // to get the QP lo and hi.
    QP->lo = G->W * std::max(0., targetSplit - tol);
    QP->hi = G->W * std::min(1., targetSplit + tol);
    ASSERT (QP->lo <= QP->hi);

    /* Convert the guess from discrete to continuous. */
    double *D = QP->D;
    double *guess = QP->x;
    bool *partition = G->partition;
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
                guess[k] = MONGOOSE_IN_BOUNDARY(k) ? 0.75 : 1.0;
            }
            else
            {
                guess[k] = MONGOOSE_IN_BOUNDARY(k) ? 0.25 : 0.0;
            }
        }
        double maxWeight = -INFINITY;
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
        QP->lambda = QPnapsack(guess, n, QP->lo, QP->hi, G->w, QP->lambda,
                               QP->FreeSet_status,
                               QP->wx[1], QP->wi[0], QP->wi[1]);
    }

    // Build the FreeSet, compute grad, possibly adjust QP->lo and QP->hi
    if (!QPlinks(G, O, QP))
    {
        Logger::toc(QPTiming);
        return false;
    }

    // lo <= a'x <= hi now holds (lo and hi are modified as needed in QPLinks)

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
                std::min(cost.W[0], cost.W[1]) / G->W;

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
                partition[k]
            );

            if (externalDegree[k] > 0) bhInsert(G, k);
        }
    }

    // clear the marks from all the nodes
    G->clearMarkArray();

    /* Free the QP structure */
    QP->~QPDelta();
    SuiteSparse_free(QP);

    /* Write the cut cost back to the graph. */
    G->cutCost = cost.cutCost;
    G->W0 = cost.W[0];
    G->W1 = cost.W[1];
    G->imbalance = cost.imbalance;
    double absImbalance = fabs(G->imbalance);
    G->heuCost = G->cutCost +
                 (absImbalance > O->tolerance ? absImbalance * G->H : 0.0);

    Logger::toc(QPTiming);

    return true;
}

} // end namespace Mongoose
