
#include "mongoose_debug.hpp"

namespace Mongoose
{

/* debug_Print */
void print(cs *G)
{
    Int *Gp = G->p; Int *Gi = G->i; double *Gx = G->x;

    for(Int j = 0; j < G->n; j++)
    {
        for(int p = Gp[j]; p < Gp[j+1]; p++)
        {
            printf("G(%ld,%ld) = %f\n", Gi[p], j, Gx[p]);
        }
    }
}

void print(Graph *G)
{

}

} // end namespace Mongoose
