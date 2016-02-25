
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
     * Generate a guess cut and do FM refinement.
     * On failure, unwind the stack.
     */
     /*
    if(!guessCut(current, O))
    {
         while(current != G)
         {
              Graph *next = current->parent;
              current->~Graph();
              SuiteSparse_free(current);
              current = next;
         }
         return;
    }
    */
    /*
     * Refine the guess cut back to the beginning.
     */
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

    // Create variables
    csi n = G->n;
    double *c = G->w;
    // Can optimize mallocs here later
    // UB/LB may be redundant with GRB_BINARY
    double *x_lb = (double*)SuiteSparse_malloc(n, sizeof(double));
    double *x_ub = (double*)SuiteSparse_malloc(n, sizeof(double));
    double *y_lb = (double*)SuiteSparse_malloc(n, sizeof(double));
    double *y_ub = (double*)SuiteSparse_malloc(n, sizeof(double));
    char *types = (char*)SuiteSparse_malloc(n, sizeof(char));
    for (int i = 0; i < n; i++) {
      x_lb[i] = 0;
      x_ub[i] = 1;
      y_lb[i] = 0;
      y_ub[i] = 1;
      types[i] = GRB_BINARY;
    }
    GRBVar* x_vars = model.addVars (x_lb, x_ub, c, types, NULL, n);
    GRBVar* y_vars = model.addVars (y_lb, y_ub, c, types, NULL, n);

    // Integrate new variables

    model.update();
    // Set objective: maximize c'*(x+y)
    GRBLinExpr obj = GRBLinExpr (0.0);
    obj.addTerms (c, x_vars, n);
    obj.addTerms (c, y_vars, n);
    model.setObjective(obj, GRB_MAXIMIZE);

    // Add constraints: x_i + y_i <= 1
    for (int i = 0; i < n; i++)
    {
      model.addConstr(x_vars[i] + y_vars[i], GRB_LESS_EQUAL, 1.0);
    }

    // Add constraints: lb <= sum(x) <= ub
    GRBLinExpr sum_x = GRBLinExpr (0.0);
    GRBLinExpr sum_y = GRBLinExpr (0.0);
    for(int i = 0; i < n; i++)
    {
      sum_x += c[i]*x_vars[i];
      sum_y += c[i]*y_vars[i];
    }
    double ub = 2.0/3.0*G->W;
    double lb = 1;
    model.addConstr(sum_x, GRB_LESS_EQUAL, ub);
    model.addConstr(sum_y, GRB_LESS_EQUAL, ub);
    model.addConstr(sum_x, GRB_GREATER_EQUAL, lb);
    model.addConstr(sum_y, GRB_GREATER_EQUAL, lb);

    // Optimize model
    //model.update();
    //model.write("check.lp");
    model.optimize();
    /*
    for(int i = 0; i < n; i++)
    {
      cout << "x[" << i << "] = " << x_vars[i].get(GRB_DoubleAttr_X)*G->w[i];
      cout << ", y[" << i << "] = " << y_vars[i].get(GRB_DoubleAttr_X)*G->w[i] << endl;
    }
    cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
    */
    for(int i = 0; i < n; i++)
    {
      G->partition[i] = (x_vars[i].get(GRB_DoubleAttr_X) > 0.9)? 0 : 1;
      G->separator[i] = (y_vars[i].get(GRB_DoubleAttr_X) > 0.9)? 0 : 1;
    }
  } catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }
}

}