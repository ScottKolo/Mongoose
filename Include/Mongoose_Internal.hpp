#ifndef Mongoose_Internal_hpp
#define Mongoose_Internal_hpp

#ifndef MONGOOSE_ONE
#define MONGOOSE_ONE 1.0
#endif

#ifndef MONGOOSE_ZERO
#define MONGOOSE_ZERO 0.0
#endif

#ifndef MONGOOSE_MIN2
#define MONGOOSE_MIN2(x,y) ((x)<(y) ? (x) : (y))
#endif

#ifndef MONGOOSE_MAX2
#define MONGOOSE_MAX2(x,y) ((x)>(y) ? (x) : (y))
#endif

#include <climits>
#include <cstdlib>

/* Dependencies */
#include "assert.h"
#include "stddef.h"
#include "stdlib.h"
#include "math.h"

/* Memory Management */
#include "SuiteSparse_config.h"

/* Debug and Timing Management */
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

/* Type definitions */
typedef long Int;
typedef double Weight;  /* Used for floating point edge & node weights */
typedef double Double;  /* Used for floating point other calculations  */

/* Enumerations */
enum MatchingStrategy{ Random, HEM, HEMPA, HEMDavisPA, LabelPropagation };
enum GuessCutType{ Pseudoperipheral_Fast, Pseudoperipheral_All, QP_GradProj, QP_BallOpt };

} // end namespace Mongoose

#endif