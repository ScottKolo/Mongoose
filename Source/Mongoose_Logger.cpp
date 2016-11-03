
#include "Mongoose_Logger.hpp"
#include <iostream>

namespace Mongoose
{

int Logger::debugLevel = None;
bool Logger::timingOn = false;
clock_t Logger::clocks[6];
float Logger::times[6];

void Logger::setDebugLevel(int debugType)
{
    debugLevel = debugType;
}

void Logger::setTimingFlag(bool tFlag)
{
    timingOn = tFlag;
}

void Logger::printTimingInfo()
{
    printf(" Matching:   %0.3fs\n", times[MatchingTiming]);
    printf(" Coarsening: %0.3fs\n", times[CoarseningTiming]);
    printf(" Refinement: %0.3fs\n", times[RefinementTiming]);
    printf(" FM:         %0.3fs\n", times[FMTiming]);
    printf(" QP:         %0.3fs\n", times[QPTiming]);
    printf(" IO:         %0.3fs\n", times[IOTiming]);
}

} // end namespace Mongoose