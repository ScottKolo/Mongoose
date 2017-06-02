
#include "Mongoose_Internal.hpp"
#include "Mongoose_Options.hpp"
#include "Mongoose_Debug.hpp"
#include "Mongoose_Logger.hpp"

namespace Mongoose
{

/* Constructor & Destructor */
Options* Options::Create()
{
    Options *ret = static_cast<Options*>(SuiteSparse_calloc(1, sizeof(Options)));
    
    if(ret != NULL)
    {
        ret->randomSeed = 0;

        ret->coarsenLimit = 256;
        ret->matchingStrategy = HEMDavisPA;
        ret->doCommunityMatching = false;
        ret->davisBrotherlyThreshold = 2.0;

        ret->guessCutType = Pseudoperipheral_Fast;
        ret->guessSearchDepth = 10;

        ret->numDances = 1;

        ret->useFM = true;
        ret->fmSearchDepth = 50;
        ret->fmConsiderCount = 3;
        ret->fmMaxNumRefinements = 20;

        ret->useQPGradProj = true;
        ret->gradProjTolerance = 0.001;
        ret->gradprojIterationLimit = 50;

        ret->targetSplit = 0.5;
        ret->softSplitTolerance = 0;
    }

    return ret;
}

} // end namespace Mongoose
