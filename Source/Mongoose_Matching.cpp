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
#include <map>

namespace Mongoose
{

//-----------------------------------------------------------------------------
// top-level matching code that serves as a multiple-dispatch system.
//-----------------------------------------------------------------------------
void match(Graph *G, Options *O)
{
    Logger::tic(MatchingTiming);
    switch (O->matchingStrategy)
    {
      case Random:
          matching_Random(G,O);
          matching_Cleanup(G,O);
          break;

      case HEM:
          matching_HEM(G,O);
          matching_Cleanup(G,O);
          break;

      case HEMPA:
          matching_HEM(G,O);
          matching_PA(G,O);
          if (!O->doCommunityMatching) matching_Cleanup(G,O);
          break;

      case HEMDavisPA:
          matching_HEM(G,O);
          matching_DavisPA(G,O);
          matching_Cleanup(G,O);
          break;

//      case LabelPropagation:
//          matching_LabelProp(G,O);
//          matching_Cleanup(G,O);
//          break;

    }
    Logger::toc(MatchingTiming);
}

//-----------------------------------------------------------------------------
// Cleans up a matching by matching remaining unmatched vertices to themselves
//-----------------------------------------------------------------------------
void matching_Cleanup(Graph *G, Options *O)
{
    Int n = G->n;
    Int cn = G->cn;
    Int *matching = G->matching;
    Int *matchmap = G->matchmap;
    Int *invmatchmap = G->invmatchmap;
    Int *matchtype = G->matchtype;

    /* Match unmatched vertices to themselves. */
    for (Int k = 0; k < n; k++)
    {
        if (!MONGOOSE_IS_MATCHED(k)) { MONGOOSE_MATCH(k, k, MatchType_Orphan); }
    }

    /* Save the # of coarse nodes. */
    G->cn = cn;
}

//-----------------------------------------------------------------------------
// This is a random matching strategy
//-----------------------------------------------------------------------------
void matching_Random(Graph *G, Options *O)
{
    Int n = G->n;
    Int cn = G->cn;
    Int *Gp = G->p;
    Int *Gi = G->i;
    Int *matching = G->matching;
    Int *matchmap = G->matchmap;
    Int *invmatchmap = G->invmatchmap;
    Int *matchtype = G->matchtype;

    for (Int k = 0; k < n; k++)
    {
        /* Consider only unmatched nodes */
        if (MONGOOSE_IS_MATCHED(k)) continue;

        bool unmatched = true;
        for (Int p = Gp[k]; p < Gp[k+1] && unmatched; p++)
        {
            Int neighbor = Gi[p];

            /* Consider only unmatched neighbors */
            if (MONGOOSE_IS_MATCHED(neighbor)) continue;

            unmatched = false;

            MONGOOSE_MATCH(k, neighbor, MatchType_Standard);
        }
    }

    /* Save the # of coarse nodes. */
    G->cn = cn;

#ifndef NDEBUG
    /* If we want to do expensive checks, make sure that every node is either:
     *     1) matched
     *     2) has no unmatched neighbors
     */
    if (O->doExpensiveChecks)
    {
        for (Int k = 0; k < n; k++)
        {
            /* Check condition 1 */
            if (matching[k]) continue;
            /* Check condition 2 */
            for (Int p = Gp[k]; p < Gp[k+1]; p++)
            {
                ASSERT (matching[Gi[p]]);
            }
        }
    }
#endif

}

//-----------------------------------------------------------------------------
// This is the implementation of passive-aggressive matching
//-----------------------------------------------------------------------------
void matching_PA(Graph *G, Options *O)
{
    Int n = G->n;
    Int cn = G->cn;
    Int *Gp = G->p;
    Int *Gi = G->i;
    Weight *Gx = G->x;
    Int *matching = G->matching;
    Int *matchmap = G->matchmap;
    Int *invmatchmap = G->invmatchmap;
    Int *matchtype = G->matchtype;

    /* In order for us to use Passive-Aggressive matching,
     * all unmatched vertices must have matched neighbors. */
    if (O->doExpensiveChecks)
    {
        for (Int k = 0; k < n; k++)
        {
            if (MONGOOSE_IS_MATCHED(k)) continue;
            for (Int p = Gp[k]; p < Gp[k+1]; p++)
            {
                ASSERT (MONGOOSE_IS_MATCHED(Gi[p]));
            }
        }
    }

    for (Int k = 0; k < n; k++)
    {
        /* Consider only unmatched nodes */
        if (MONGOOSE_IS_MATCHED(k)) continue;

        Int heaviestNeighbor = -1;
        Weight heaviestWeight = -1.0;

        for (Int p = Gp[k]; p < Gp[k+1]; p++)
        {
            Int neighbor = Gi[p];

            /* Keep track of the heaviest. */
            Weight x = Gx[p];
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
                if (MONGOOSE_IS_MATCHED(neighbor)) continue;

                if (v == -1)
                {
                    v = neighbor;
                }
                else
                {
                    MONGOOSE_MATCH(v, neighbor, MatchType_Brotherly);
                    v = -1;
                }
            }

            /* If we had a vertex left over: */
            if (v != -1)
            {
                if (O->doCommunityMatching)
                {
                    MONGOOSE_COMMUNITY_MATCH(heaviestNeighbor, v,
                                             MatchType_Community);
                }
                else
                {
                    MONGOOSE_MATCH(v, v, MatchType_Orphan);
                }
            }
        }
    }

    /* Save the # of coarse nodes. */
    G->cn = cn;

    /* Every vertex must be matched in no more than a 3-way matching. */
    if (O->doExpensiveChecks)
    {
        for (Int k = 0; k < n; k++)
        {
            if (O->doCommunityMatching)
            {
                if (!MONGOOSE_IS_MATCHED(k)) PR (("%ld is unmatched\n", k)) ;
                ASSERT (MONGOOSE_IS_MATCHED(k));
            }

            /* Load matching. */
            Int v[3] = {-1, -1, -1};
            v[0] = k;
            v[1] = MONGOOSE_GETMATCH(v[0]);
            if (v[1] == v[0]) v[1] = -1;
            if (v[1] != -1)
            {
                v[2] = MONGOOSE_GETMATCH(v[1]);
                if (v[2] == v[0]) v[2] = -1;
            }

            if (O->doCommunityMatching)
            {
                if (v[2] != -1) { ASSERT (MONGOOSE_GETMATCH(v[2]) == v[0]); }
                else            { ASSERT (MONGOOSE_GETMATCH(v[1]) == v[0]); }
            }
            else
            {
                if (v[1] != -1) { ASSERT (MONGOOSE_GETMATCH(v[1]) == v[0]); }
                else            { ASSERT (MONGOOSE_GETMATCH(v[0]) == v[0]); }
            }
        }
    }
}

//-----------------------------------------------------------------------------
// This uses the Davis style passive-aggressive matching where we only try
// PA matching if the problem is some percent unmatched.
//-----------------------------------------------------------------------------
void matching_DavisPA(Graph *G, Options *O)
{
    Int n = G->n;
    Int cn = G->cn;
    Int *Gp = G->p;
    Int *Gi = G->i;
    Int *matching = G->matching;
    Int *matchmap = G->matchmap;
    Int *invmatchmap = G->invmatchmap;
    Int *matchtype = G->matchtype;

    /* The brotherly threshold is the Davis constant times average degree. */
    Weight bt = O->davisBrotherlyThreshold * ((Weight) G->nz / (Weight) G->n);

    /* In order for us to use Passive-Aggressive matching,
     * all unmatched vertices must have matched neighbors. */
    if (O->doExpensiveChecks)
    {
        for (Int k = 0; k < n; k++)
        {
            if (MONGOOSE_IS_MATCHED(k)) continue;
            for (Int p = Gp[k]; p < Gp[k+1]; p++)
            {
                ASSERT (MONGOOSE_IS_MATCHED(Gi[p]));
            }
        }
    }

    for (Int k = 0; k < n; k++)
    {
        /* Consider only matched nodes */
        if (!MONGOOSE_IS_MATCHED(k)) continue;

        Int degree = Gp[k+1] - Gp[k];
        if (degree >= (Int) bt)
        {
            Int v = -1;
            for (Int p = Gp[k]; p < Gp[k+1]; p++)
            {
                Int neighbor = Gi[p];
                if (MONGOOSE_IS_MATCHED(neighbor)) continue;

                if (v == -1)
                {
                    v = neighbor;
                }
                else
                {
                    MONGOOSE_MATCH(v, neighbor, MatchType_Brotherly);
                    v = -1;
                }
            }

            /* If we had a vertex left over: */
            if (v != -1)
            {
                if (O->doCommunityMatching)
                {
                    MONGOOSE_COMMUNITY_MATCH(k, v, MatchType_Community);
                }
                else
                {
                    MONGOOSE_MATCH(v, v, MatchType_Orphan);
                }
            }
        }
    }

    /* Save the # of coarse nodes. */
    G->cn = cn;
    ASSERT (cn < n);
}

//-----------------------------------------------------------------------------
// This is a vanilla implementation of heavy edge matching
//-----------------------------------------------------------------------------
void matching_HEM(Graph *G, Options *O)
{
    Int n = G->n;
    Int cn = G->cn;
    Int *Gp = G->p;
    Int *Gi = G->i;
    Weight *Gx = G->x;
    Int *matching = G->matching;
    Int *matchmap = G->matchmap;
    Int *invmatchmap = G->invmatchmap;
    Int *matchtype = G->matchtype;

    for (Int k = 0; k < n; k++)
    {
        /* Consider only unmatched nodes */
        if (MONGOOSE_IS_MATCHED(k)) continue;

        Int heaviestNeighbor = -1;
        Weight heaviestWeight = -1.0;
        for (Int p = Gp[k]; p < Gp[k + 1]; p++)
        {
            Int neighbor = Gi[p];

            /* Consider only unmatched neighbors */
            if (MONGOOSE_IS_MATCHED(neighbor)) continue;

            /* Keep track of the heaviest. */
            Weight x = Gx[p];
            if (x > heaviestWeight)
            {
                heaviestWeight = x;
                heaviestNeighbor = neighbor;
            }
        }

        /* Match to the heaviest. */
        if (heaviestNeighbor != -1)
        {
            MONGOOSE_MATCH(k, heaviestNeighbor, MatchType_Standard);
        }
    }

    /* Save the # of coarse nodes. */
    G->cn = cn;

    /* If we want to do expensive checks, make sure that every node is either:
     *     1) matched
     *     2) has no unmatched neighbors
     */
    if (O->doExpensiveChecks)
    {
        for (Int k = 0; k < n; k++)
        {
            /* Check condition 1 */
            if (matching[k]) continue;

            /* Check condition 2 */
            for (Int p = Gp[k]; p < Gp[k + 1]; p++)
            {
                ASSERT (matching[Gi[p]]);
            }
        }
    }
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
            if (MONGOOSE_IS_MATCHED(k)) continue;

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
                MONGOOSE_MATCH(k, mostFrequentLabel, MatchType_Community);
                numMatched += 1;
            } else {
                label[k] = mostFrequentLabel;
            }
        }
    }

    SuiteSparse_free(label);

    // Save the # of coarse nodes.
    G->cn = cn;

    //  If we want to do expensive checks, make sure that every node is either:
    //     1) matched
    //     2) has no unmatched neighbors

    if (O->doExpensiveChecks)
    {
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
    }
}
#endif

} // end namespace Mongoose
