#include "Mongoose_Internal.hpp"
#include "Mongoose_GuessCut.hpp"
#include "Mongoose_ImproveQP.hpp"
#include "Mongoose_Waterdance.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

Int diagBFS
(
    Graph *G,
    Options *O,
    Int *stack,
    Int *mark,
    Int markStart,
    Int *inout_start
);

void partBFS
(
    Graph *G,
    Options *O,
    Int start
);

void findAllPseudoperipheralNodes
(
    Graph *G,
    Options *O,
    Int *list,
    Int *listsize,
    bool *ppvMark
);

void pseudoperipheralGuess
(
    Graph *G,
    Options *O
);

//-----------------------------------------------------------------------------
// This function takes a graph with options and computes the initial guess cut
//-----------------------------------------------------------------------------
bool guessCut(Graph *G, Options *O)
{
    switch (O->guessCutType)
    {
      case Pseudoperipheral_All:
      {
          bool *ppvMark = (bool*) SuiteSparse_calloc(G->n, sizeof(bool));
          if (!ppvMark) return false;

          /* Find pseudoperipheral nodes from which we can generate guess cuts. */
          Int *list = G->matching, size = 0;
          findAllPseudoperipheralNodes(G, O, list, &size, ppvMark);

          /* Find the best guess. */
          Weight bestCost = INFINITY;
          Int bestGuess = -1;

          for (Int i = 0; i < size; i++)
          {
              /* Generate guess partition using a BFS from the start node and do FM on it. */
              partBFS(G, O, list[i]);
              bhLoad(G, O);
              waterdance(G, O);
              bhClear(G);

              if (G->cutCost < bestCost)
              {
                  bestGuess = i;
                  bestCost = G->cutCost;
              }
          }

          /* Load the best guess. */
          partBFS(G, O, list[bestGuess]);
          bhLoad(G, O);

          SuiteSparse_free(ppvMark);
          break;
      }

      default: case Pseudoperipheral_Fast:
      {
          pseudoperipheralGuess(G, O);
          break;
      }

      case QP_GradProj:
      {
          // TODO will this work?  Or will it break lo <= a'*x <= hi?
          for (Int k = 0; k < G->n; k++) G->partition[k] = false;
          G->W0 = G->W;
          G->W1 = 0.0;
          bhLoad(G, O);
          improveCutUsingQP(G, O, true);
          break;
      }
    }

    /* Do the waterdance refinement. */
    waterdance(G, O);

    return true;
}

//-----------------------------------------------------------------------------
// diagBFS returns the diagonal and proposes a new start vertex.
//-----------------------------------------------------------------------------
Int diagBFS
(
    Graph *G,
    Options *O,
    Int *stack,
    Int *mark,
    Int markStart,
    Int *inout_start
)
{
    Int n = G->n;
    bool *partition = G->partition;
    Int *Gp = G->p;
    Int *Gi = G->i;
    Weight *Gw = G->w;

    Int markValue = markStart;
    Int start = *inout_start;

    Weight halfW = G->W / 2.0;
    Weight W0 = 0.0;

    Int head = 0, tail = 0;
    stack[tail++] = start;
    MONGOOSE_MARK(start);

    Int v, vmark;
    while (head < tail)
    {
        v = stack[head++];
        W0 += Gw[v];
        partition[v] = (W0 < halfW);
        vmark = mark[v]+1;

        for (Int p = Gp[v]; p < Gp[v+1]; p++)
        {
            Int neighbor = Gi[p];
            if (mark[neighbor] < markStart)
            {
                mark[neighbor] = vmark;
                stack[tail++] = neighbor;
            }
        }
    }

    Int maxLevel = vmark - 1;
    Int diameter = maxLevel - markStart - 1;

    /* Choose the next start based on the smallest degree. */
    Int minDegree = n;
    for (Int v = stack[--head]; mark[v] == maxLevel && minDegree > 1;
         v = stack[--head])
    {
        Int degree = Gp[v+1] - Gp[v];
        if (degree < minDegree)
        {
            minDegree = degree;
            *inout_start = v;
        }
    }

    return diameter;
}

//-----------------------------------------------------------------------------
// partBFS builds a guess partition using BFS region growing from a start node
//-----------------------------------------------------------------------------
void partBFS
(
    Graph *G,
    Options *O,
    Int start
)
{
    bool *partition = G->partition;
    Int *Gp = G->p;
    Int *Gi = G->i;
    Weight *Gw = G->w;

    Int *mark = G->mark;
    Int markValue = G->markValue;

    Weight halfW = G->W / 2.0;
    Weight W0 = 0.0;

    Int *stack = G->matchmap, head = 0, tail = 0;
    stack[tail++] = start;
    MONGOOSE_MARK(start);

    while (head < tail)
    {
        Int v = stack[head++];
        W0 += Gw[v];
        partition[v] = (W0 < halfW);

        for (Int p = Gp[v]; p < Gp[v+1]; p++)
        {
            Int neighbor = Gi[p];
            if (!MONGOOSE_MARKED(neighbor))
            {
                MONGOOSE_MARK(neighbor);
                stack[tail++] = neighbor;
            }
        }
    }

    // clear the marks from all the nodes
    MONGOOSE_CLEAR_ALL_MARKS ;      // TODO: reset if int overflow
    G->markValue = markValue ;
}

//-----------------------------------------------------------------------------
// This function finds all of the pseudoperipheral nodes
// It is used as a kernel for a guess cut code that tries region-growing
// from every pseudoperipheral node
//-----------------------------------------------------------------------------
void findAllPseudoperipheralNodes
(
    Graph *G,
    Options *O,
    Int *list,
    Int *listsize,
    bool *ppvMark
)
{
    Int n = G->n;
    Int *mark = G->mark;
    Int markValue = G->markValue;
    Int diameter = 0;

    Int *stack = G->matchmap, head = 0, tail = 0;

    /* Add the first vertex to the list. */
    Int startVertex = O->randomSeed % n;
    list[tail++] = startVertex;
    ppvMark[startVertex] = true;

    /* Do a number of BFSs in order to find a pseudoperipheral node. */
    Int i = 0, guessSearchDepth = O->guessSearchDepth;
    while (head != tail)
    {
        if (i++ > guessSearchDepth) break;

        // TODO make sure markValue + (n+1) does not overflow
        // and reset if it does, BEFORE using the markValue

        Int start = list[head++];
        diameter = diagBFS(G, O, stack, mark, markValue, &start);

        // clear all marks
        markValue += diameter + 1;      // TODO: reset if int overflow

        /* Go backwards through the last level and add new vertices to the list
         * of pseudoperipheral nodes that we're finding. */
        for (Int s = n-1; s >= 0; s--)
        {
            Int v = stack[s];
            if (!MONGOOSE_MARKED(v)) break;

            if (!ppvMark[v])
            {
                list[tail++] = v;
                ppvMark[v] = true;
            }
        }

        MONGOOSE_CLEAR_ALL_MARKS ;      // TODO: reset if int overflow
    }

    // clear the marks from all the nodes
    MONGOOSE_CLEAR_ALL_MARKS ;      // TODO: reset if int overflow
    G->markValue = markValue ;

    *listsize = tail;
}

//-----------------------------------------------------------------------------
// This function finds a pseudoperipheral node
// It is used as a kernel for a guess cut code that tries region-growing
// from just one pseudoperipheral node
//-----------------------------------------------------------------------------
void pseudoperipheralGuess
(
    Graph *G,
    Options *O
)
{
    Int n = G->n;
    Int *mark = G->mark;
    Int markValue = G->markValue;

    Int *stack = G->matching;
    for (Int k = 0; k < n; k++) stack[k] = 0;

    Int start = O->randomSeed % n;
    Int diameter = -1, newDiameter = 0;

    /* Do a number of BFSs in order to find a pseudoperipheral node. */
    Int guessSearchDepth = O->guessSearchDepth;
    for (Int i = 0; i < guessSearchDepth || diameter < newDiameter; i++)
    {

        // TODO make sure markValue + (n+1) does not overflow
        // and reset if it does, BEFORE using the markValue

        diameter = newDiameter;
        newDiameter = diagBFS(G, O, stack, mark, markValue, &start);

        // clear all marks
        markValue += newDiameter + 2;       // TODO reset if int overflow
    }

    // clear the marks from all the nodes
    MONGOOSE_CLEAR_ALL_MARKS ;      // TODO: reset if int overflow
    G->markValue = markValue ;

    /* Load the boundary heap. */
    bhLoad(G, O);
}

} // end namespace Mongoose
