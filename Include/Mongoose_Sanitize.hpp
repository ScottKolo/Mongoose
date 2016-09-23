#ifndef Mongoose_Sanitize_hpp
#define Mongoose_Sanitize_hpp

#include "Mongoose_CSparse.hpp"
#include "Mongoose_Internal.hpp"

namespace Mongoose
{

cs *sanitizeMatrix(cs *compressed_A, bool symmetric_triangular);
void removeDiagonal(cs *A);
// Requires A to be a triangular matrix with no diagonal.
cs *mirrorTriangular(cs *A);

} // end namespace Mongoose

#endif