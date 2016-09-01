#ifndef Mongoose_Debug_hpp
#define Mongoose_Debug_hpp

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_CSparse.hpp"
#include "Mongoose_Interop.hpp"

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
