#include "Mongoose_Internal.hpp"
#include "Mongoose_GuessCut.hpp"
#include "Mongoose_ImproveQP.hpp"
#include "Mongoose_Waterdance.hpp"

namespace Mongoose
{

//-----------------------------------------------------------------------------
// This function takes a graph with options and computes the initial guess cut
//-----------------------------------------------------------------------------
bool guessCut(Graph *graph, Options *options)
{
    switch (options->guessCutType)
    {
      case Pseudoperipheral_All:
      {
          /* Find pseudoperipheral nodes from which we can generate guess cuts. */
          Int *list = graph->matching, size = 0;
          if (!findAllPseudoperipheralNodes(graph, options, list, &size)) return false;

          /* Find the best guess. */
          double bestCost = INFINITY;
          Int bestGuess = -1;

          for (Int i = 0; i < size; i++)
          {
              /* Generate guess partition using a BFS from the start node and do FM on it. */
              partBFS(graph, options, list[i]);
              bhLoad(graph, options);
              waterdance(graph, options);
              bhClear(graph);

              if (graph->cutCost < bestCost)
              {
                  bestGuess = i;
                  bestCost = graph->cutCost;
              }
          }

          /* Load the best guess. */
          partBFS(graph, options, list[bestGuess]);
          bhLoad(graph, options);
          break;
      }

      default: case Pseudoperipheral_Fast:
      {
          pseudoperipheralGuess(graph, options);
          break;
      }

      case QP_GradProj:
      {
          for (Int k = 0; k < graph->n; k++) graph->partition[k] = false;
          graph->W0 = graph->W;
          graph->W1 = 0.0;
          bhLoad(graph, options);
          if (!improveCutUsingQP(graph, options, true))
          {
              return false;
              // Error - QP Failure
          }
          break;
      }
    }

    /* Do the waterdance refinement. */
    waterdance(graph, options);

    return true;
}

//-----------------------------------------------------------------------------
// diagBFS returns the diagonal and proposes a new start vertex.
//-----------------------------------------------------------------------------
Int diagBFS
(
    Graph *graph,
    Options *options,
    Int *stack,
    Int *inout_start
)
{
    (void)options; // Unused variable

    Int n = graph->n;
    bool *partition = graph->partition;
    Int *Gp = graph->p;
    Int *Gi = graph->i;
    double *Gw = graph->w;

    graph->checkForSpaceAndResetIfNeeded(n+1);
    Int markStart = graph->getMarkValue();
    Int start = *inout_start;

    double halfW = graph->W / 2.0;
    double W0 = 0.0;

    Int head = 0, tail = 0;
    stack[tail++] = start;
    graph->mark(start);

    Int vmark;
    while (head < tail)
    {
        Int v = stack[head++];
        W0 += Gw[v];
        partition[v] = (W0 < halfW);
        vmark = graph->getMarkArrayValue(v)+1;

        for (Int p = Gp[v]; p < Gp[v+1]; p++)
        {
            Int neighbor = Gi[p];
            if (graph->getMarkArrayValue(neighbor) < markStart)
            {
                graph->mark(neighbor, vmark);
                stack[tail++] = neighbor;
            }
        }
    }

    Int maxLevel = vmark - 1;
    Int diameter = maxLevel - markStart - 1;

    /* Choose the next start based on the smallest degree. */
    Int minDegree = n;
    for (Int v = stack[--head]; graph->getMarkArrayValue(v) == maxLevel && minDegree > 1;
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
    Graph *graph,
    Options *options,
    Int start
)
{
    (void)options; // Unused variable

    bool *partition = graph->partition;
    Int *Gp = graph->p;
    Int *Gi = graph->i;
    double *Gw = graph->w;

    double halfW = graph->W / 2.0;
    double W0 = 0.0;

    Int *stack = graph->matchmap, head = 0, tail = 0;
    stack[tail++] = start;
    graph->mark(start);

    while (head < tail)
    {
        Int v = stack[head++];
        W0 += Gw[v];
        partition[v] = (W0 < halfW);

        for (Int p = Gp[v]; p < Gp[v+1]; p++)
        {
            Int neighbor = Gi[p];
            if (!graph->isMarked(neighbor))
            {
                graph->mark(neighbor);
                stack[tail++] = neighbor;
            }
        }
    }

    // clear the marks from all the nodes
    graph->clearMarkArray();
}

//-----------------------------------------------------------------------------
// This function finds all of the pseudoperipheral nodes
// It is used as a kernel for a guess cut code that tries region-growing
// from every pseudoperipheral node
//-----------------------------------------------------------------------------
bool findAllPseudoperipheralNodes
(
    Graph *graph,
    Options *options,
    Int *list,
    Int *listsize
)
{
    Int n = graph->n;

    bool *ppvMark = (bool*) SuiteSparse_calloc(graph->n, sizeof(bool));
    if (!ppvMark) return false;

    Int *stack = graph->matchmap, head = 0, tail = 0;

    /* Add the first vertex to the list. */
    Int startVertex = std::rand() % n;
    list[tail++] = startVertex;
    ppvMark[startVertex] = true;

    /* Do a number of BFSs in order to find a pseudoperipheral node. */
    Int i = 0, guessSearchDepth = options->guessSearchDepth;
    while (head != tail)
    {
        if (i++ > guessSearchDepth) break;

        Int start = list[head++];
        Int diameter = diagBFS(graph, options, stack, &start);

        // clear all marks
        graph->clearMarkArray(diameter + 1);

        /* Go backwards through the last level and add new vertices to the list
         * of pseudoperipheral nodes that we're finding. */
        for (Int s = n-1; s >= 0; s--)
        {
            Int v = stack[s];
            if (!graph->isMarked(v)) break;

            if (!ppvMark[v])
            {
                list[tail++] = v;
                ppvMark[v] = true;
            }
        }

        graph->clearMarkArray();
    }

    // clear the marks from all the nodes
    graph->clearMarkArray();

    *listsize = tail;

    SuiteSparse_free(ppvMark);

    return true;
}

//-----------------------------------------------------------------------------
// This function finds a pseudoperipheral node
// It is used as a kernel for a guess cut code that tries region-growing
// from just one pseudoperipheral node
//-----------------------------------------------------------------------------
void pseudoperipheralGuess
(
    Graph *graph,
    Options *options
)
{
    Int n = graph->n;

    Int *stack = graph->matching;
    for (Int k = 0; k < n; k++) stack[k] = 0;

    Int start = options->randomSeed % n;
    Int diameter = -1, newDiameter = 0;

    /* Do a number of BFSs in order to find a pseudoperipheral node. */
    Int guessSearchDepth = options->guessSearchDepth;
    for (Int i = 0; i < guessSearchDepth || diameter < newDiameter; i++)
    {
        diameter = newDiameter;
        newDiameter = diagBFS(graph, options, stack, &start);

        // clear all marks
        graph->clearMarkArray(newDiameter + 2);
    }

    // clear the marks from all the nodes
    graph->clearMarkArray();

    /* Load the boundary heap. */
    bhLoad(graph, options);
}

} // end namespace Mongoose
