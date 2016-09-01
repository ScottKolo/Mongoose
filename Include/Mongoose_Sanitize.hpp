#ifndef Mongoose_Sanitize_hpp
#define Mongoose_Sanitize_hpp

#include "Mongoose_CSparse.hpp"
#include "Mongoose_Internal.hpp"

namespace Mongoose
{

cs *sanitize_matrix(cs *compressed_A, bool symmetric_triangular);
void get_triangular(cs *A, bool lower);
void remove_diagonal(cs *A);
// Requires A to be a triangular matrix with no diagonal.
cs *mirror_triangular(cs *A);

} // end namespace Mongoose

#endif