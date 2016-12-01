/**
 * @file Mongoose_Logger.cpp
 * @author Scott Kolodziej
 * @date 23 Sep 2016
 * @brief Centralized debug and timing manager
 *
 * @details For debug and timing information to be displayed via stdout. This system
 * allows this information to be displayed (or not) without recompilation.
 * Timing inforation for different *portions of the library are also managed 
 * here with a tic/toc pattern.
 */

#include "Mongoose_Logger.hpp"
#include <iostream>
#include <cstdio>

namespace Mongoose
{

int Logger::debugLevel = None;
bool Logger::timingOn = false;
clock_t Logger::clocks[6];
float Logger::times[6];

std::ostream* Logger::error_ = NULL;
std::ostream* Logger::warn_  = NULL;
std::ostream* Logger::info_  = NULL;
std::ostream* Logger::test_  = NULL;

void Logger::setDebugLevel(int debugType)
{
    debugLevel = debugType;

    Logger::error_ = (debugLevel & Error) ? &std::cerr : NULL;
    Logger::warn_ =  (debugLevel & Warn)  ? &std::cout : NULL;
    Logger::info_ =  (debugLevel & Info)  ? &std::cout : NULL;
    Logger::test_ =  (debugLevel & Test)  ? &std::cout : NULL;
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