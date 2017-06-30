/**
 * For computing vertex matchings
 *
 * During coarsening, a matching of vertices is computed to determine
 * which vertices are combined together into supernodes. This can be done using
 * a number of different strategies, including Heavy Edge Matching and 
 * Community/Brotherly (similar to 2-hop) Matching.
 */

#include "Mongoose_Internal.hpp"
#include "Mongoose_Matching.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

//-----------------------------------------------------------------------------
// top-level matching code that serves as a multiple-dispatch system.
//-----------------------------------------------------------------------------
void match(Graph *graph, Options *options)
{
    Logger::tic(MatchingTiming);
    switch (options->matchingStrategy)
    {
      case Random:
          matching_Random(graph,options);
          matching_Cleanup(graph,options);
          break;

      case HEM:
          matching_HEM(graph,options);
          matching_Cleanup(graph,options);
          break;

      case HEMPA:
          matching_HEM(graph,options);
          matching_PA(graph,options);
          if (!options->doCommunityMatching) matching_Cleanup(graph,options);
          break;

      case HEMDavisPA:
          matching_HEM(graph,options);
          matching_DavisPA(graph,options);
          matching_Cleanup(graph,options);
          break;

//      case LabelPropagation:
//          matching_LabelProp(graph,options);
//          matching_Cleanup(graph,options);
//          break;

    }
    Logger::toc(MatchingTiming);
}

//-----------------------------------------------------------------------------
// Cleans up a matching by matching remaining unmatched vertices to themselves
//-----------------------------------------------------------------------------
void matching_Cleanup(Graph *graph, Options *options)
{
    (void)options; // Unused variable

    Int n = graph->n;
    Int *Gp = graph->p;

    /* Match unmatched vertices to themselves. */
    for (Int k = 0; k < n; k++)
    {
        if (!graph->isMatched(k))
        {
            Int degree = Gp[k + 1] - Gp[k];
            if (degree == 0)
            {
                // Singleton!
                if (graph->singleton == -1)
                {
                    graph->singleton = k;
                }
                else
                {
                    graph->createMatch(k, graph->singleton, MatchType_Standard);
                    graph->singleton = -1;
                }
            }
            else
            {
                // Not a singleton
                graph->createMatch(k, k, MatchType_Orphan);
            }
        }
    }

    if (graph->singleton != -1)
    {
        // Leftover singleton
        Int k = graph->singleton;
        graph->createMatch(k, k, MatchType_Orphan);
    }
}

//-----------------------------------------------------------------------------
// This is a random matching strategy
//-----------------------------------------------------------------------------
void matching_Random(Graph *graph, Options *options)
{
    (void)options; // Unused variable

    Int n = graph->n;
    Int *Gp = graph->p;
    Int *Gi = graph->i;

    for (Int k = 0; k < n; k++)
    {
        /* Consider only unmatched nodes */
        if (graph->isMatched(k)) continue;

        bool unmatched = true;
        for (Int p = Gp[k]; p < Gp[k+1] && unmatched; p++)
        {
            Int neighbor = Gi[p];

            /* Consider only unmatched neighbors */
            if (graph->isMatched(neighbor)) continue;

            unmatched = false;

            graph->createMatch(k, neighbor, MatchType_Standard);
        }
    }

#ifndef NDEBUG
    /* If we want to do expensive checks, make sure that every node is either:
     *     1) matched
     *     2) has no unmatched neighbors
     */
    for (Int k = 0; k < n; k++)
    {
        /* Check condition 1 */
        if (graph->matching[k]) continue;
        /* Check condition 2 */
        for (Int p = Gp[k]; p < Gp[k+1]; p++)
        {
            ASSERT (graph->matching[Gi[p]]);
        }
    }
#endif

}

//-----------------------------------------------------------------------------
// This is the implementation of passive-aggressive matching
//-----------------------------------------------------------------------------
void matching_PA(Graph *graph, Options *options)
{
    Int n = graph->n;
    Int *Gp = graph->p;
    Int *Gi = graph->i;
    double *Gx = graph->x;

#ifndef NDEBUG
    /* In order for us to use Passive-Aggressive matching,
     * all unmatched vertices must have matched neighbors. */
    for (Int k = 0; k < n; k++)
    {
        if (graph->isMatched(k)) continue;
        for (Int p = Gp[k]; p < Gp[k+1]; p++)
        {
            ASSERT (graph->isMatched(Gi[p]));
        }
    }
#endif

    for (Int k = 0; k < n; k++)
    {
        /* Consider only unmatched nodes */
        if (graph->isMatched(k)) continue;

        Int heaviestNeighbor = -1;
        double heaviestWeight = -1.0;

        for (Int p = Gp[k]; p < Gp[k+1]; p++)
        {
            Int neighbor = Gi[p];

            /* Keep track of the heaviest. */
            double x = Gx[p];
            if (x > heaviestWeight)
            {
                heaviestWeight = x;
                heaviestNeighbor = neighbor;
            }
        }

        /* If we found a heaviest neighbor then begin resolving matches. */
        if (heaviestNeighbor != -1)
        {
            Int v = -1;
            for (Int p = Gp[heaviestNeighbor]; p < Gp[heaviestNeighbor+1]; p++)
            {
                Int neighbor = Gi[p];
                if (graph->isMatched(neighbor)) continue;

                if (v == -1)
                {
                    v = neighbor;
                }
                else
                {
                    graph->createMatch(v, neighbor, MatchType_Brotherly);
                    v = -1;
                }
            }

            /* If we had a vertex left over: */
            if (v != -1)
            {
                if (options->doCommunityMatching)
                {
                    graph->createCommunityMatch(heaviestNeighbor, v,
                                             MatchType_Community);
                }
                else
                {
                    graph->createMatch(v, v, MatchType_Orphan);
                }
            }
        }
    }

#ifndef NDEBUG
    /* Every vertex must be matched in no more than a 3-way matching. */
    for (Int k = 0; k < n; k++)
    {
        if (options->doCommunityMatching)
        {
            if (!graph->isMatched(k)) PR (("%ld is unmatched\n", k)) ;
            ASSERT (graph->isMatched(k));
        }

        /* Load matching. */
        Int v[3] = {-1, -1, -1};
        v[0] = k;
        v[1] = graph->getMatch(v[0]);
        if (v[1] == v[0]) v[1] = -1;
        if (v[1] != -1)
        {
            v[2] = graph->getMatch(v[1]);
            if (v[2] == v[0]) v[2] = -1;
        }

        if (options->doCommunityMatching)
        {
            if (v[2] != -1) { ASSERT (graph->getMatch(v[2]) == v[0]); }
            else            { ASSERT (graph->getMatch(v[1]) == v[0]); }
        }
        else
        {
            if (v[1] != -1) { ASSERT (graph->getMatch(v[1]) == v[0]); }
            else            { ASSERT (graph->getMatch(v[0]) == v[0]); }
        }
    }
#endif

}

//-----------------------------------------------------------------------------
// This uses the Davis style passive-aggressive matching where we only try
// PA matching if the problem is some percent unmatched.
//-----------------------------------------------------------------------------
void matching_DavisPA(Graph *graph, Options *options)
{
    Int n = graph->n;
    Int *Gp = graph->p;
    Int *Gi = graph->i;

    /* The brotherly threshold is the Davis constant times average degree. */
    double bt = options->davisBrotherlyThreshold * ((double) graph->nz / (double) graph->n);

#ifndef NDEBUG
    /* In order for us to use Passive-Aggressive matching,
     * all unmatched vertices must have matched neighbors. */
    for (Int k = 0; k < n; k++)
    {
        if (graph->isMatched(k)) continue;
        for (Int p = Gp[k]; p < Gp[k+1]; p++)
        {
            ASSERT (graph->isMatched(Gi[p]));
        }
    }
#endif

    for (Int k = 0; k < n; k++)
    {
        /* Consider only matched nodes */
        if (!graph->isMatched(k)) continue;

        Int degree = Gp[k+1] - Gp[k];
        if (degree >= (Int) bt)
        {
            Int v = -1;
            for (Int p = Gp[k]; p < Gp[k+1]; p++)
            {
                Int neighbor = Gi[p];
                if (graph->isMatched(neighbor)) continue;

                if (v == -1)
                {
                    v = neighbor;
                }
                else
                {
                    graph->createMatch(v, neighbor, MatchType_Brotherly);
                    v = -1;
                }
            }

            /* If we had a vertex left over: */
            if (v != -1)
            {
                if (options->doCommunityMatching)
                {
                    graph->createCommunityMatch(k, v, MatchType_Community);
                }
                else
                {
                    graph->createMatch(v, v, MatchType_Orphan);
                }
            }
        }
    }

    ASSERT (graph->cn < n);
}

//-----------------------------------------------------------------------------
// This is a vanilla implementation of heavy edge matching
//-----------------------------------------------------------------------------
void matching_HEM(Graph *graph, Options *options)
{
    (void)options; // Unused variable

    Int n = graph->n;
    Int *Gp = graph->p;
    Int *Gi = graph->i;
    double *Gx = graph->x;

    for (Int k = 0; k < n; k++)
    {
        /* Consider only unmatched nodes */
        if (graph->isMatched(k)) continue;

        Int heaviestNeighbor = -1;
        double heaviestWeight = -1.0;
        for (Int p = Gp[k]; p < Gp[k + 1]; p++)
        {
            Int neighbor = Gi[p];

            /* Consider only unmatched neighbors */
            if (graph->isMatched(neighbor)) continue;

            /* Keep track of the heaviest. */
            double x = Gx[p];
            if (x > heaviestWeight)
            {
                heaviestWeight = x;
                heaviestNeighbor = neighbor;
            }
        }

        /* Match to the heaviest. */
        if (heaviestNeighbor != -1)
        {
            graph->createMatch(k, heaviestNeighbor, MatchType_Standard);
        }
    }

#ifndef NDEBUG
    /* If we want to do expensive checks, make sure that every node is either:
     *     1) matched
     *     2) has no unmatched neighbors
     */
    for (Int k = 0; k < n; k++)
    {
        /* Check condition 1 */
        if (graph->matching[k]) continue;

        /* Check condition 2 */
        for (Int p = Gp[k]; p < Gp[k + 1]; p++)
        {
            ASSERT (graph->matching[Gi[p]]);
        }
    }
#endif

}

//-----------------------------------------------------------------------------
// This is a label propagation strategy
//-----------------------------------------------------------------------------

#if 0
void matching_LabelProp(Graph *G, Options *O)
{
    Int n = G->n;
    Int cn = G->cn;
    Int *Gp = G->p;
    Int *Gi = G->i;
    Int *matching = G->matching;
    Int *matchmap = G->matchmap;
    Int *invmatchmap = G->invmatchmap;
    Int *matchtype = G->matchtype;

    Int *label;
    label = (Int *)SuiteSparse_malloc(n, sizeof(Int));

    for (Int k = 0; k < n; k++)
    {
        label[k] = k;
    }

    Int numMatched = 0;
    while (numMatched < n)
    {
        for (Int k = 0; k < n; k++)
        {
            // Consider only unmatched nodes
            if (graph->isMatched(k)) continue;

            std::map<Int, Int> count;
            Int max = 0;
            Int mostFrequentLabel = -1;

            for (Int p = Gp[k]; p < Gp[k + 1]; p++) {
                // Compute most frequent label
                Int currentCount = count[label[Gi[p]]] + 1;
                count[label[Gi[p]]] = currentCount;
                if (currentCount > max) {
                    max = currentCount;
                    mostFrequentLabel = label[Gi[p]];
                }
            }

            if (label[k] == mostFrequentLabel) {
                graph->createMatch(k, mostFrequentLabel, MatchType_Community);
                numMatched += 1;
            } else {
                label[k] = mostFrequentLabel;
            }
        }
    }

    SuiteSparse_free(label);

    // Save the # of coarse nodes.
    G->cn = cn;

#ifndef NDEBUG
    //  If we want to do expensive checks, make sure that every node is either:
    //     1) matched
    //     2) has no unmatched neighbors
    for (Int k = 0; k < n; k++)
    {
        // Check condition 1
        if (matching[k]) continue;

        // Check condition 2
        for (Int p = Gp[k]; p < Gp[k+1]; p++)
        {
            ASSERT (matching[Gi[p]]);
        }
    }
#endif

}
#endif

} // end namespace Mongoose
