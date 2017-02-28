#ifndef Mongoose_QPBoundary_hpp
#define Mongoose_QPBoundary_hpp

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_QPDelta.hpp"

namespace Mongoose
{

void QPcheckCom
(
    Graph *G,
    Options *O,
    QPDelta *QP,
    bool check_b,
    Int nFreeSet,
    Double b
) ;

void FreeSet_dump (const char *where,
    Int n, Int *LinkUp, Int *LinkDn, Int nFreeSet, Int *FreeSet_status,
    Int verbose, Double *x) ;

void QPBoundary
(
    Graph *G,
    Options *O,
    QPDelta *QP
);

} // end namespace Mongoose

#endif
