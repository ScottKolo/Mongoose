#ifndef Mongoose_Test_hpp
#define Mongoose_Test_hpp

#include <string>

void runIOTest(const std::string inputFile, bool validGraph);
void runMemoryTest(const std::string inputFile);
void runTimingAndResultsTests();
void runTimingAndResultsTest(const std::string inputFile);
void runEdgeSeparatorTest(const std::string inputFile);

#endif
