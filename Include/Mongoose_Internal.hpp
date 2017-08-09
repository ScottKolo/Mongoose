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
enum MatchingStrategy
{
    Random,
    HEM,
    HEMPA,
    HEMDavisPA
};

enum GuessCutType
{
    GuessQP,
    GuessRandom,
    GuessNaturalOrder
};

enum MatchType
{
    MatchType_Orphan = 0,
    MatchType_Standard = 1,
    MatchType_Brotherly = 2,
    MatchType_Community = 3
};

} // end namespace Mongoose

