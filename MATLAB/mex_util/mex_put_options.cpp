#include "mongoose_mex.hpp"

namespace Mongoose
{

#define MEX_STRUCT_PUT(F)        addFieldWithValue(returner, #F, (double) O->F);

mxArray *mex_put_options
(
    const Options *O
)
{
    mxArray *returner = mxCreateStructMatrix(1, 1, 0, NULL);

    MEX_STRUCT_PUT(randomSeed);
    MEX_STRUCT_PUT(coarsenLimit);
    MEX_STRUCT_PUT(matchingStrategy);
    MEX_STRUCT_PUT(doCommunityMatching);
    MEX_STRUCT_PUT(highDegreeThreshold);
    
    /** Guess Partitioning Options *******************************************/
    MEX_STRUCT_PUT(guessCutType);

    /** Waterdance Options ***************************************************/
    MEX_STRUCT_PUT(numDances);

    /**** Fidducia-Mattheyes Options *****************************************/
    MEX_STRUCT_PUT(useFM);
    MEX_STRUCT_PUT(fmSearchDepth);
    MEX_STRUCT_PUT(fmConsiderCount);
    MEX_STRUCT_PUT(fmMaxNumRefinements);

    /**** Quadratic Programming Options **************************************/
    MEX_STRUCT_PUT(useQPGradProj);
    MEX_STRUCT_PUT(gradProjTolerance);
    MEX_STRUCT_PUT(gradprojIterationLimit);

    /** Final Partition Target Metrics ***************************************/
    MEX_STRUCT_PUT(targetSplit);
    MEX_STRUCT_PUT(softSplitTolerance);

    return returner;
}

}
