// This include file should not be visible to the user

#pragma once

#define FREESET_DEBUG 0

#if __cplusplus > 199711L
#define CPP11_OR_LATER true
#else
#define CPP11_OR_LATER false
#endif

#include <climits>
#include <cstdlib>

/* Dependencies */
#include "math.h"
#include "stddef.h"
#include "stdlib.h"

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
enum MatchingStrategy
{
    Random     = 0,
    HEM        = 1,
    HEMPA      = 2,
    HEMDavisPA = 3
};

enum GuessCutType
{
    GuessQP           = 0,
    GuessRandom       = 1,
    GuessNaturalOrder = 2
};

enum MatchType
{
    MatchType_Orphan    = 0,
    MatchType_Standard  = 1,
    MatchType_Brotherly = 2,
    MatchType_Community = 3
};

} // end namespace Mongoose
