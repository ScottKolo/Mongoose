#ifndef HAGER_HPP_
#define HAGER_HPP_

#include "mongoose_internal.hpp"
#include "mongoose_qpdelta.hpp"
#include "mongoose_gradproj.hpp"

namespace SuiteSparse_Mongoose
{

void qpGradProj
(
    Graph *G,
    Options *O,
    bool isInitial = false
);

void qpBallOpt
(
    Graph *G,
    Options *O
);

}

#endif
