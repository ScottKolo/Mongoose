/**
 * Coarsening of a graph given a previously determined matching
 *
 * In order to operate on extremely large graphs, a pre-processing is
 * done to reduce the size of the graph while maintaining its overall structure.
 * Given a matching of vertices with other vertices (e.g. heavy edge matching, 
 * random, etc.), coarsening constructs the new, coarsened graph.
 */

#include "Mongoose_Internal.hpp"
#include "Mongoose_Coarsening.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

/**
 * @brief Coarsen a Graph given a previously calculated matching
 *
 * Given a Graph @p G, coarsen returns a new Graph that is coarsened according
 * to the matching given by G->matching, G->matchmap, and G->invmatchmap.
 * G->matching must be built such that matching[a] = b+1 and matching[b] = a+1
 * if vertices a and b are matched. G->matchmap is a mapping from fine to coarse
 * vertices, so matchmap[a] = matchmap[b] = c if vertices a and b are matched
 * and mapped to vertex c in the coarse graph. Likewise, G->invmatchmap is
 * one possible inverse of G->matchmap, so invmatchmap[c] = a or
 * invmatchmap[c] = b if a coarsened vertex c represents the matching of
 * vertices a and b in the refined graph.
 *
 * @code
 * Graph coarsened_graph = coarsen(large_graph, options);
 * @endcode
 *
 * @param graph Graph to be coarsened
 * @param options Option struct specifying if debug checks should be done
 * @return A coarsened version of G
 * @note Allocates memory for the coarsened graph, but frees on error.
 */
Graph *coarsen(Graph *graph, Options *options)
{
    (void)options; // Unused variable

    Logger::tic(CoarseningTiming);

    Int cn = graph->cn;
    Int *Gp = graph->p;
    Int *Gi = graph->i;
    double *Gx = graph->x;
    double *Gw = graph->w;

    Int *matchmap = graph->matchmap;
    Int *invmatchmap = graph->invmatchmap;

    /* Build the coarse graph */
    Graph *coarseGraph = Graph::Create(graph);
    if (!coarseGraph) return NULL;

    Int *Cp = coarseGraph->p;
    Int *Ci = coarseGraph->i;
    double *Cx = coarseGraph->x;
    double *Cw = coarseGraph->w;
    double *gains = coarseGraph->vertexGains;
    Int munch = 0;
    double X = 0.0;

    /* Hashtable stores column pointer values. */
    Int *htable = (Int*) SuiteSparse_malloc(static_cast<size_t>(cn), sizeof(Int));
    if (!htable)
    {
        coarseGraph->~Graph();
        SuiteSparse_free(coarseGraph);
        return NULL;
    }
    for (Int i = 0; i < cn; i++) htable[i] = -1;

    /* For each vertex in the coarse graph. */
    for (Int k = 0; k < cn; k++)
    {
        /* Load up the inverse matching */
        Int v[3] = {-1, -1, -1};
        v[0] = invmatchmap[k];
        v[1] = graph->getMatch(v[0]);
        if (v[0] == v[1]) { v[1] = -1; }
        else
        {
            v[2] = graph->getMatch(v[1]);
            if (v[0] == v[2]) { v[2] = -1; }
        }

        Int ps = Cp[k] = munch;     /* The munch start for this column */

        double nodeWeight = 0.0;
        double sumEdgeWeights = 0.0;
        for (Int i = 0; i < 3 && v[i] != -1; i++)
        {
            /* Read the matched vertex and accumulate the node weight. */
            Int vertex = v[i];
            nodeWeight += Gw[vertex];

            for (Int p = Gp[vertex]; p < Gp[vertex+1]; p++)
            {
                Int toCoarsened = matchmap[Gi[p]];
                if (toCoarsened == k) continue;         /* Delete self-edges */

                /* Read the edge weight and accumulate the sum of edge weights. */
                double edgeWeight = Gx[p];
                sumEdgeWeights += Gx[p];

                /* Check the hashtable before scattering. */
                Int cp = htable[toCoarsened];
                if (cp < ps)             /* Hasn't been seen yet this column */
                {
                    htable[toCoarsened] = munch;
                    Ci[munch] = toCoarsened;
                    Cx[munch] = edgeWeight;
                    munch++;
                }
                /* If the entry already exists & we have edge weights,
                 * sum the edge weights here. */
                else
                {
                    Cx[cp] += edgeWeight;
                }
            }
        }

        /* Save the node weight. */
        Cw[k] = nodeWeight;

        /* Save the sum of edge weights and initialize the gain for k. */
        X += sumEdgeWeights;
        gains[k] = -sumEdgeWeights;
    }

    /* Set the last column pointer */
    Cp[cn] = munch;
    coarseGraph->nz = munch;

    /* Save the sum of edge weights on the graph. */
    coarseGraph->X = X;
    coarseGraph->H = 2.0 * X;

    coarseGraph->worstCaseRatio = graph->worstCaseRatio;

    /* Cleanup resources */
    SuiteSparse_free(htable);

#ifndef NDEBUG
    /* If we want to do expensive checks, make sure we didn't break
     * the problem into multiple connected components. */
    double W = 0.0;
    for (Int k = 0; k < cn; k++)
    {
        W += Cw[k];
    }
    ASSERT (W == coarseGraph->W) ;
#endif

    Logger::toc(CoarseningTiming);

    /* Return the coarse graph */
    return coarseGraph;
}

} // end namespace Mongoose
