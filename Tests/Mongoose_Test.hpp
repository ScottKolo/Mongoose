#ifndef Mongoose_Test_hpp
#define Mongoose_Test_hpp

#include <climits>
#include <cstdlib>
#include <cassert>

/* Dependencies */
#include "stddef.h"
#include "stdlib.h"
#include "math.h"

/* Memory Management */
#include "SuiteSparse_config.h"

#include <string>

void runIOTest(const std::string &inputFile, bool validGraph);
void runMemoryTest(const std::string &inputFile);
void runTimingTest(const std::string &inputFile);
void runEdgeSeparatorTest(const std::string &inputFile, const double targetSplit);
void runPerformanceTest(const std::string &inputFile, const std::string &outputFile);

// Currently unused
void runReferenceTest(const std::string &inputFile);

#include "Mongoose_Logger.hpp"

#endif
