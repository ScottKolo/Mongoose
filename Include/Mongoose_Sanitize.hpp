#pragma once

#include "Mongoose_CSparse.hpp"
#include "Mongoose_Internal.hpp"

namespace Mongoose
{

cs *sanitizeMatrix(cs *compressed_A, bool symmetricTriangular, bool makeEdgeWeightsBinary);
void removeDiagonal(cs *A);
// Requires A to be a triangular matrix with no diagonal.
cs *mirrorTriangular(cs *A);

} // end namespace Mongoose
