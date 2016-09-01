#ifndef Mongoose_ImproveQP_hpp
#define Mongoose_ImproveQP_hpp

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_QPDelta.hpp"
#include "Mongoose_QPGradProj.hpp"

namespace Mongoose
{

void improveCutUsingQP
(
    Graph *G,
    Options *O,
    bool isInitial = false
);

} // end namespace Mongoose

#endif
