/**
 * @file Mongoose_Logger.hpp
 * @author Scott Kolodziej
 * @date 23 Sep 2016
 * @brief Centralized debug and timing manager
 *
 * For debug and timing information to be displayed via stdin. This system
 * allows this information to be displayed (or not) without recompilation.
 */

#ifndef Mongoose_Logger_hpp
#define Mongoose_Logger_hpp

#include <string>
#include <time.h> 
#include <iostream>

namespace Mongoose
{

enum DebugType
{
    None = 0,
    Error = 1,
    Warn = 2,
    Info = 4
} typedef DebugType;

enum TimingType
{
    NoTiming = 0,
    MatchingTiming = 1,
    CoarseningTiming = 2,
    RefinementTiming = 4,
    FMTiming = 8,
    QPTiming = 16,
    IOTiming = 32
} typedef TimingType;

class Logger
{
  private:
    static DebugType debugLevel;
    static TimingType timingLevel;

  public:
    Logger();
    Logger(DebugType dType, TimingType tType);
    static inline void log(DebugType debugType, std::string output);
    static void setDebugLevel(DebugType debugType);
    static void setTimingLevel(TimingType timingType);
};

inline void Logger::log(DebugType debugType, std::string output)
{
    if (debugType & debugLevel)
    {
        std::cout << output << std::endl;
    }
}

} // end namespace Mongoose

#endif