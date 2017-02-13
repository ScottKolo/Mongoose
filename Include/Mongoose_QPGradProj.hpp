#ifndef Mongoose_QPGradProj_hpp
#define Mongoose_QPGradProj_hpp

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_QPDelta.hpp"

namespace Mongoose
{

void QPlinks
(
    Graph *G,
    Options *O,
    QPDelta *QP          /* pointer to QPDelta structure  */
);

Double QPgradproj
(
    Graph *G,
    Options *O,
    QPDelta *QP
);

void QPboundary
(
    Graph *G,
    Options *O,
    QPDelta *QP
);

inline void saveContext
(
    Graph *G,
    QPDelta *QP, 
    Int it,
    Double err,
    Int nf,
    Int ib,
    Double lo,
    Double hi
)
{
    QP->its = it;
    QP->err = err;
    QP->numFreeVars = nf;
    
    Double b = 0.0;
    if(ib != 0)
    {
        b = (ib > 0 ? hi : lo);
    }
    else
    {
        for (Int k = 0; k < G->n; k++) b += G->w[k] * QP->x[k];
    }
    QP->ib = ib;
    QP->b = b;
}

} // end namespace Mongoose

#endif
