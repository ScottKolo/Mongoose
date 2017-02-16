#ifndef Mongoose_Debug_hpp
#define Mongoose_Debug_hpp

#include "Mongoose_Graph.hpp"
#include "Mongoose_CSparse.hpp"

#include <cassert>

// Mongoose Logic Macros
#ifndef MONGOOSE_IMPLIES
#define MONGOOSE_IMPLIES(p,q)    (!(p) || ((p) && (q)))
#endif
#ifndef MONGOOSE_IFF
#define MONGOOSE_IFF(p,q)        (IMPLIES(p,q) && IMPLIES(q,p))
#endif

// turn off debugging
#ifndef NDEBUG
#define NDEBUG
#endif

// Uncomment this line to enable debugging.  Mongoose will be very slow.
// #undef NDEBUG

#undef ASSERT
#ifndef NDEBUG
#define ASSERT(expression) (assert (expression))
#define ASSERT_TEXT(expression, text) (assert (expression && text))
#else
#define ASSERT(expression)
#define ASSERT_TEXT(expression, text)
#endif

namespace Mongoose
{

/* debug_Print */
//void print(cs *G);
//void print(Graph *G);

} // end namespace Mongoose

#endif
