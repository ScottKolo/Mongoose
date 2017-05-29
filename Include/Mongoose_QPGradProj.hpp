#ifndef Mongoose_QPGradProj_hpp
#define Mongoose_QPGradProj_hpp

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_QPDelta.hpp"

namespace Mongoose
{

double QPgradproj
(
    Graph *G,
    Options *O,
    QPDelta *QP
);

} // end namespace Mongoose

#endif
