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
    Info = 4,
    Test = 8,
    All = 15
} typedef DebugType;

enum TimingType
{
    MatchingTiming = 0,
    CoarseningTiming = 1,
    RefinementTiming = 2,
    FMTiming = 3,
    QPTiming = 4,
    IOTiming = 5
} typedef TimingType;

class Logger
{
  private:
    static int debugLevel;
    static bool timingOn;
    static clock_t clocks[6];
    static float times[6];

  public:
    Logger();
    Logger(DebugType dType, bool tFlag);
    static inline void log(DebugType debugType, std::string output);
    static inline void tic(TimingType timingType);
    static inline void toc(TimingType timingType);
    static inline float getTime(TimingType timingType);
    static void setDebugLevel(int debugType);
    static void setTimingFlag(bool tFlag);
    static void printTimingInfo();
};

inline void Logger::log(DebugType debugType, std::string output)
{
    if (debugType & debugLevel)
    {
        std::cout << output << "\n";
    }
}

inline void Logger::tic(TimingType timingType)
{
    if (timingOn)
    {
        clocks[timingType] = clock();
    }
}

inline void Logger::toc(TimingType timingType)
{
    if (timingOn)
    {
        times[timingType] += ((float)(clock() - clocks[timingType])) / CLOCKS_PER_SEC;
    }
}

inline float Logger::getTime(TimingType timingType)
{
    return times[timingType];
}

} // end namespace Mongoose

#endif