
#include "mongoose_internal.hpp"
#include "mongoose_coarsening.hpp"
#include "mongoose_guesscut.hpp"
#include "mongoose_refining.hpp"
#include "mongoose_waterdance.hpp"

#include "gurobi_c++.h"
#include <iostream>

using namespace std;

namespace SuiteSparse_Mongoose
{

void milp_refine(Graph *G, Options *O);
vector<Int> reduceProblem(Graph *G, Int hops, vector<Int> &red_to_orig, Int &red_n);

/* The input must be a single connected component. */
void ComputeVertexSeparator(Graph *G, Options *O)
{
    /* Check inputs */
    if(!G || !O) return;

    /* Finish initialization */
    if(!initialize(G, O)) return;

    /* Keep track of what the current graph is at any stage */
    Graph *current = G;

    /* If we need to coarsen the graph, do the coarsening. */
    while(current->n >= O->coarsenLimit)
    {
        match(current, O);
        Graph *next = coarsen(current, O);
        /* If we ran out of memory during coarsening, unwind the stack. */
        if(!next)
        {
             while(current != G)
             {
                  next = current->parent;
                  current->~Graph();
                  SuiteSparse_free(current);
                  current = next;
             }
             return;
        }

        current = next;
    }

    /*
     * Refine the guess cut back to the beginning.
     */
    milp_refine(current, O);
    while(current->parent != NULL)
    {
      current = refine(current, O);
      milp_refine(current, O);
    }
}

void milp_refine(Graph *G, Options *O)
{
  printf("Beginning MILP Refinement\n");
  
  try {
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env);
    model.getEnv().set("OutputFlag", "0");
    //model.getEnv().set("MIPGapAbs", "0");
    model.set(GRB_IntAttr_ModelSense, -1);

    // Massive heuristic optimization
    // Only add vertices within 3 hops of the separator

    Int n = G->n;
    Int red_n;
    vector<Int> red_to_orig;
    vector<Int> orig_to_red = reduceProblem(G, 3, red_to_orig, red_n);

    // Create variables
    double *c = G->w;
    double *red_c = (double*)SuiteSparse_malloc(red_n, sizeof(double));
    char *types = (char*)SuiteSparse_malloc(red_n, sizeof(char));

    for (Int i = 0; i < red_n; i++)
    {
      types[i] = GRB_BINARY;
      red_c[i] = c[red_to_orig[i]];
    }
    GRBVar* x_vars = model.addVars (NULL, NULL, red_c, types, NULL, red_n);
    GRBVar* y_vars = model.addVars (NULL, NULL, red_c, types, NULL, red_n);

    // Integrate new variables
    model.update();

    // Initialize variables (warm start)
    for (Int i = 0; i < red_n; i++)
    {
      x_vars[i].set(GRB_DoubleAttr_Start, (G->partition[red_to_orig[i]])?1:0);
      y_vars[i].set(GRB_DoubleAttr_Start, (G->separator[red_to_orig[i]])?1:0);
    }

    // Add constraints: x_i + y_j <= 1 for all (i,j) in E
    // O(nnz)
    for (Int j = 0 ; j < red_n ; j++)
    {
        Int col = red_to_orig[j];
        for (Int p = G->p [col] ; p < G->p [col+1] ; p++)
        {
          if(orig_to_red[G->i[p]] >= 0)
          {
            model.addConstr(x_vars[j] + y_vars[orig_to_red[G->i[p]]], GRB_LESS_EQUAL, 1.0);
            model.addConstr(x_vars[orig_to_red[G->i[p]]] + y_vars[j], GRB_LESS_EQUAL, 1.0);
          }
          else if (G->partition[G->i[p]] && !G->separator[G->i[p]])
          {
            model.addConstr(x_vars[j], GRB_LESS_EQUAL, 1.0);
            model.addConstr(y_vars[j], GRB_EQUAL, 0);
          }
          else if (!G->partition[G->i[p]] && G->separator[G->i[p]])
          {
            model.addConstr(x_vars[j], GRB_EQUAL, 0);
            model.addConstr(y_vars[j], GRB_LESS_EQUAL, 1.0);
          }
        }
    }

    // Add constraints: x_i + y_i <= 1 for i in 1:n
    for (Int j = 0 ; j < red_n ; j++)
    {
        model.addConstr(x_vars[j] + y_vars[j], GRB_LESS_EQUAL, 1.0);
    }

    // Add constraints: lb <= sum(x) <= ub
    // O(n)
    Int x_wgts = 0;
    Int y_wgts = 0;
    for(Int i = 0; i < n; i++)
    {
      if(G->partition[i] && !G->separator[i])
      {
        x_wgts += G->w[i];
      }
      else if(!G->partition[i] && G->separator[i])
      {
        y_wgts += G->w[i];
      }
    }
    GRBLinExpr sum_x = GRBLinExpr (x_wgts);
    GRBLinExpr sum_y = GRBLinExpr (y_wgts);
    for(Int i = 0; i < red_n; i++)
    {
      sum_x += red_c[i]*x_vars[i];
      sum_y += red_c[i]*y_vars[i];
    }
    double ub = 2.0/3.0*G->W;
    double lb = 1;
    model.addConstr(sum_x, GRB_LESS_EQUAL, ub);
    model.addConstr(sum_y, GRB_LESS_EQUAL, ub);
    model.addConstr(sum_x, GRB_GREATER_EQUAL, lb);
    model.addConstr(sum_y, GRB_GREATER_EQUAL, lb);

    // Output model to file - Debugging
    //model.update();
    //model.write("check" +to_string(G->clevel) + ".lp");

    // Optimize model
    model.optimize();

    Int sepsize = 0;
    for(Int i = 0; i < red_n; i++)
    {
      G->partition[red_to_orig[i]] = (x_vars[i].get(GRB_DoubleAttr_X) < 0.5)? false : true;
      G->separator[red_to_orig[i]] = (y_vars[i].get(GRB_DoubleAttr_X) < 0.5)? false : true;
      if(!G->partition[red_to_orig[i]] && !G->separator[red_to_orig[i]])
        sepsize++;
    }
    printf("Sep size: %f %ld\n", red_n - model.get(GRB_DoubleAttr_ObjVal), sepsize);

    // Free allocated variables
    red_c = (double*)SuiteSparse_free(red_c);
    types = (char*)SuiteSparse_free(types);
  } catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }
}

// Reduce the problem to include only the separator plus vertices within a
// certain distance (hops). Return the reduced number of vertices (red_n),
// the mapping of reduced vertices to original vertices (red_to_orig), and
// the mapping of original vertices to reduced vertices (orig_to_red, function
// return).
vector<Int> reduceProblem(Graph *G, Int hops, vector<Int> &red_to_orig, Int &red_n)
{
  Int *Gp = G->p;
  Int *Gi = G->i;
  Int n = G->n;
  red_n = 0;

  vector<Int> red_index(n, -1);
  Int count = 0;
  for(Int i = 0; i < n; i++)
  {
    if(!G->partition[i] && !G->separator[i])
    {
      red_index[i] = count;
      red_to_orig.push_back(i);
      count++;
    }
  }

  Int start = 0;
  Int end = count;
  for(Int k = 0; k < hops-1; k++)
  {
    start = end;
    end = count;
    for(Int i = start; i < end; i++)
    {
      Int j = red_to_orig[i];
      for(Int p = Gp[j]; p < Gp[j+1]; p++)
      {
        if(red_index[Gi[p]] >= 0)
        {
          continue;
        }
        else
        {
          red_index[Gi[p]] = count;
          red_to_orig.push_back(Gi[p]);
          count++;
        }
      }
    }
  }
  red_n = count;
  return red_index;
}

}