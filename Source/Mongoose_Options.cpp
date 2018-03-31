/* ========================================================================== */
/* === Source/Mongoose_Options.cpp ========================================== */
/* ========================================================================== */

/* -----------------------------------------------------------------------------
 * Mongoose Graph Partitioning Library  Copyright (C) 2017-2018,
 * Scott P. Kolodziej, Nuri S. Yeralan, Timothy A. Davis, William W. Hager
 * Mongoose is licensed under Version 3 of the GNU General Public License.
 * Mongoose is also available under other licenses; contact authors for details.
 * -------------------------------------------------------------------------- */

#include "Mongoose_Options.hpp"
#include "Mongoose_Internal.hpp"

namespace Mongoose
{

/* Constructor & Destructor */
Options *Options::Create()
{
    Options *ret
        = static_cast<Options *>(SuiteSparse_malloc(1, sizeof(Options)));

    if (ret != NULL)
    {
        ret->randomSeed = 0;

        ret->coarsenLimit            = 64;
        ret->matchingStrategy        = HEMSR;
        ret->doCommunityMatching     = false;
        ret->highDegreeThreshold     = 2.0;

        ret->guessCutType = GuessRandom;

        ret->numDances = 1;

        ret->useFM               = true;
        ret->fmSearchDepth       = 50;
        ret->fmConsiderCount     = 3;
        ret->fmMaxNumRefinements = 20;

        ret->useQPGradProj          = true;
        ret->gradProjTolerance      = 0.001;
        ret->gradprojIterationLimit = 50;

        ret->targetSplit        = 0.5;
        ret->softSplitTolerance = 0;
    }

    return ret;
}

Options::~Options()
{
    SuiteSparse_free(this);
}

} // end namespace Mongoose
