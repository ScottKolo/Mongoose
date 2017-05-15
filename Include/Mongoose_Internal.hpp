#ifndef Mongoose_Internal_hpp
#define Mongoose_Internal_hpp

#ifndef MONGOOSE_MIN2
#define MONGOOSE_MIN2(x,y) ((x)<(y) ? (x) : (y))
#endif

#ifndef MONGOOSE_MAX2
#define MONGOOSE_MAX2(x,y) ((x)>(y) ? (x) : (y))
#endif

#define FREESET_DEBUG 0

#include <climits>
#include <cstdlib>

/* Dependencies */
#include "stddef.h"
#include "stdlib.h"
#include "math.h"

/* Memory Management */
#include "SuiteSparse_config.h"

namespace Mongoose
{

/* Type definitions */
typedef long Int;
#ifndef MAX_INT
#define MAX_INT LONG_MAX
#endif

typedef double Weight;  /* Used for floating point edge & node weights */

/* Enumerations */
enum MatchingStrategy{ Random, HEM, HEMPA, HEMDavisPA };
enum GuessCutType{ Pseudoperipheral_Fast, Pseudoperipheral_All, QP_GradProj, QP_BallOpt };

} // end namespace Mongoose

#endif
