#ifndef MONGOOSE_DEBUG_HPP_
#define MONGOOSE_DEBUG_HPP_

#include "mongoose_internal.hpp"
#include "mongoose_cs.hpp"

/* Mongoose Logic Macros */
#ifndef MONGOOSE_IMPLIES
#define MONGOOSE_IMPLIES(p,q)    (!(p) || ((p) && (q)))
#endif
#ifndef MONGOOSE_IFF
#define MONGOOSE_IFF(p,q)        (IMPLIES(p,q) && IMPLIES(q,p))
#endif

namespace Mongoose
{

/* debug_Print */
void print(cs *G);
void print(Graph *G);
void print(Options *O);

void printToFile(Graph *G);
void printToFile(Options *O);

/* debug_BoundaryHeap */
void bhCheckHeapProperty(Int *bhIndex, Int *bhHeap, Int bhSize, Weight *gains);

/* debug_CheckGraph */
void checkGraph(Graph *G, Options *O);
void checkBoundary(Graph *G, Options *O);

} // end namespace Mongoose

#endif
