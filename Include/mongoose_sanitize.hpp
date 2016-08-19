#ifndef MONGOOSE_SANITIZE_HPP_
#define MONGOOSE_SANITIZE_HPP_

#include "mongoose_cs.hpp"
#include "mongoose_internal.hpp"

namespace Mongoose
{

cs *sanitize_matrix(cs *compressed_A, bool symmetric_triangular);
void get_triangular(cs *A, bool lower);
void remove_diagonal(cs *A);
// Requires A to be a triangular matrix with no diagonal.
cs *mirror_triangular(cs *A);

} // end namespace Mongoose

#endif