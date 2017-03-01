#ifndef Mongoose_QPBoundary_hpp
#define Mongoose_QPBoundary_hpp

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_QPDelta.hpp"

namespace Mongoose
{

void QPBoundary
(
    Graph *G,
    Options *O,
    QPDelta *QP
);

} // end namespace Mongoose

#endif
