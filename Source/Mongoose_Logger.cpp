
#include "Mongoose_Logger.hpp"
#include <iostream>

namespace Mongoose
{

DebugType Logger::debugLevel = None;
TimingType Logger::timingLevel = NoTiming;

Logger::Logger()
{
    // Default constructor
}

Logger::Logger(DebugType dType, TimingType tType)
{
    // Easy option for turning on both debug and timing information
    debugLevel = dType;
    timingLevel = tType;
}

void Logger::setDebugLevel(DebugType debugType)
{
    debugLevel = debugType;
}

void Logger::setTimingLevel(TimingType timingType)
{
    timingLevel = timingType;
}

} // end namespace Mongoose