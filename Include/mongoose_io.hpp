#ifndef MONGOOSE_IO_HPP_
#define MONGOOSE_IO_HPP_

#include "mongoose_cs.hpp"
#include "mongoose_internal.hpp"

namespace Mongoose
{

Graph *read_graph (const char* filename);
cs *read_matrix (const char* filename);
void get_triangular(cs *A, bool lower);

} // end namespace Mongoose

#endif