#ifndef Mongoose_GuessCut_hpp
#define Mongoose_GuessCut_hpp

#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_CutCost.hpp"
#include "Mongoose_BoundaryHeap.hpp"

namespace Mongoose
{

bool guessCut(Graph *G, Options *O);

void pseudoperipheralGuess(Graph *G, Options *O);
void findAllPseudoperipheralNodes(Graph *G, Options *O, Int *list, Int *listsize);

Int diagBFS
(
    Graph *G,
    Options *O,
    Int *stack,
    Int *start
);

void partBFS
(
    Graph *G,
    Options *O,
    Int start
);

} // end namespace Mongoose

#endif
