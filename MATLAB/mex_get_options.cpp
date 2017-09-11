#include "mongoose_mex.hpp"

namespace Mongoose
{

#define MEX_STRUCT_READINT(F)    returner->F = (Int) readField(matOptions, #F);
#define MEX_STRUCT_READDOUBLE(F) returner->F = readField(matOptions, #F);
#define MEX_STRUCT_READBOOL(F)   returner->F = static_cast<bool>((readField(matOptions, #F) != 0.0));
#define MEX_STRUCT_READENUM(F,T) returner->F = (T) (readField(matOptions, #F));
    
Options *mex_get_options
(
    const mxArray *matOptions
)
{
    Options *returner = Options::Create();

    if(!returner)
        return NULL;
    if(matOptions == NULL)
        return returner;

    MEX_STRUCT_READINT(randomSeed);
    MEX_STRUCT_READINT(coarsenLimit);
    MEX_STRUCT_READENUM(matchingStrategy, MatchingStrategy);
    MEX_STRUCT_READBOOL(doCommunityMatching);
    MEX_STRUCT_READDOUBLE(davisBrotherlyThreshold);
    
    /** Guess Partitioning Options *******************************************/
    MEX_STRUCT_READENUM(guessCutType, GuessCutType);

    /** Waterdance Options ***************************************************/
    MEX_STRUCT_READINT(numDances);

    /**** Fidducia-Mattheyes Options *****************************************/
    MEX_STRUCT_READBOOL(useFM);
    MEX_STRUCT_READINT(fmSearchDepth);
    MEX_STRUCT_READINT(fmConsiderCount);
    MEX_STRUCT_READINT(fmMaxNumRefinements);

    /**** Quadratic Programming Options **************************************/
    MEX_STRUCT_READBOOL(useQPGradProj);
    MEX_STRUCT_READDOUBLE(gradProjTolerance);
    MEX_STRUCT_READINT(gradprojIterationLimit);

    /** Final Partition Target Metrics ***************************************/
    MEX_STRUCT_READDOUBLE(targetSplit);
    MEX_STRUCT_READDOUBLE(softSplitTolerance);

    return returner;
}

}