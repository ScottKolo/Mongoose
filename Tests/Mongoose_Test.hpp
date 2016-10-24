#ifndef Mongoose_Test_hpp
#define Mongoose_Test_hpp

#include <string>

void runIOTests();
int runMemoryTests();
int runMemoryTest(const std::string inputFile);
void runTimingAndResultsTests();
void runTimingAndResultsTest(const std::string inputFile);
void runEdgeSeparatorTests();
void runEdgeSeparatorTest(const std::string inputFile);

#endif
