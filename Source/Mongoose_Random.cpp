#include "Mongoose_Random.hpp"

#if CPP11_OR_LATER
#include <random>
#endif

namespace Mongoose
{

#if CPP11_OR_LATER
std::random_device rd;
std::ranlux24_base generator(rd());
std::uniform_int_distribution<> distribution;
#endif

Int random()
{
#if CPP11_OR_LATER
    // Use C++11 random object
    return distribution(generator);
#else
    // Forced to use non-reentrant std::rand
    return std::rand();
#endif
}

void setRandomSeed(Int seed)
{
#if CPP11_OR_LATER
    // Use C++11 random object
    generator.seed(static_cast<unsigned int>(seed));
#else
    // Forced to use non-reentrant std::rand
    std::srand(static_cast<unsigned int>(seed));
#endif
}

} // end namespace Mongoose
