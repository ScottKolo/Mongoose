#ifndef Mongoose_Options_hpp
#define Mongoose_Options_hpp

#include "Mongoose_Internal.hpp"

namespace Mongoose
{

struct Options
{
    Int randomSeed;

    /** Coarsening Options ***************************************************/
    Int coarsenLimit;
    MatchingStrategy matchingStrategy;
    bool doCommunityMatching;
    double davisBrotherlyThreshold;

    /** Guess Partitioning Options *******************************************/
    GuessCutType guessCutType;   /* The guess cut type to use */
    Int guessSearchDepth;        /* # of times to run pseudoperipheral node
                                    finder before settling on a guess        */

    /** Waterdance Options ***************************************************/
    Int numDances;               /* The number of interplays between FM and QP
                                    at any one coarsening level. */

    /**** Fidducia-Mattheyes Options *****************************************/
    bool useFM;                  /* Flag governing the use of FM             */
    Int fmSearchDepth;           /* The # of non-positive gain move to make  */
    Int fmConsiderCount;         /* The # of heap entries to consider        */
    Int fmMaxNumRefinements;     /* Max # of times to run FidduciaMattheyes  */

    /**** Quadratic Programming Options **************************************/
    bool useQPGradProj;          /* Flag governing the use of gradproj       */
    double gradprojTol;          /* Convergence tol for projected gradient   */
    Int gradprojIterationLimit;  /* Max # of iterations for gradproj         */

    /** Final Partition Target Metrics ***************************************/
    double targetSplit;          /* The desired split ratio (default 50/50)  */
    double tolerance;            /* The allowable tolerance (default 1%)     */

    /** Debug Options ********************************************************/
    bool doExpensiveChecks;

    /* Constructor & Destructor */
    static Options *Create();
};

} // end namespace Mongoose

#endif
