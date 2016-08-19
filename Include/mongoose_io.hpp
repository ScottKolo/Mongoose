#ifndef MONGOOSE_IO_HPP_
#define MONGOOSE_IO_HPP_

#include "mongoose_cs.hpp"
#include "mongoose_internal.hpp"
#include <string>

namespace Mongoose
{

Graph *read_graph (const std::string filename);
cs *read_matrix (const std::string filename);
Graph *read_graph (const char* filename);
cs *read_matrix (const char* filename);

} // end namespace Mongoose

#endif