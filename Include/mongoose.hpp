#ifndef MONGOOSE_HPP_
#define MONGOOSE_HPP_

/* Dependencies */
#include "assert.h"
#include "stddef.h"
#include "stdlib.h"
#include "math.h"

namespace Mongoose
{

/* Type definitions */
typedef long Int;
typedef double Weight;  /* Used for floating point edge & node weights */
typedef double Double;  /* Used for floating point other calculations  */

/* Enumerations */
enum MatchingStrategy{ Random, HEM, HEMPA, HEMDavisPA };
enum GuessCutType{ Pseudoperipheral_Fast, Pseudoperipheral_All, QP_GradProj, QP_BallOpt };

} // end namespace Mongoose

/* Structures */
#include "mongoose_graph.hpp"
#include "mongoose_options.hpp"
#include "mongoose_interop.hpp"

namespace Mongoose
{

/* Function pointers */
typedef void (*fxMatcher) (Graph *G, Options *O);

void ComputeEdgeSeparator(Graph *G, Options *O);

} // end namespace Mongoose

#endif
