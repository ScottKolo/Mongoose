// This include file should not be visible to the user

#pragma once

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
typedef SuiteSparse_long Int;
#ifndef MAX_INT
#define MAX_INT SuiteSparse_long_max
#endif

/* Enumerations */
enum MatchingStrategy{ Random, HEM, HEMPA, HEMDavisPA };
enum GuessCutType{ Pseudoperipheral_Fast, Pseudoperipheral_All, QP_GradProj, QP_BallOpt };

} // end namespace Mongoose

