/**
 * @file mongoose_io.hpp
 * @author Scott Kolodziej
 * @date 13 Jun 2016
 * @brief Simplified I/O functions for reading matrices and graphs
 *
 * For reading Matrix Market files into Mongoose, read_graph and read_matrix
 * are provided (depending on if a Graph class instance or CSparse matrix
 * instance is needed). The filename can be specified as either a const char*
 * (easier for C programmers) or std::string (easier from C++).
 */

#ifndef Mongoose_IO_hpp
#define Mongoose_IO_hpp

#include "Mongoose_CSparse.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_Graph.hpp"
#include <string>

extern "C" {
#include "mmio.h"
}

namespace Mongoose
{

/** 
 * @brief Generate a Graph from a Matrix Market file.
 * 
 * Generate a Graph class instance from a Matrix Market file. The matrix
 * contained in the file must be sparse, real, and square. If the matrix
 * is not symmetric, it will be made symmetric with (A+A')/2. If the matrix has
 * more than one connected component, the largest will be found and the rest
 * discarded. If a diagonal is present, it will be removed.
 */ 
Graph *read_graph (const std::string filename);

/** 
 * @brief Generate a CSparse matrix from a Matrix Market file.
 * 
 * Generate a cs struct instance from a Matrix Market file. The matrix
 * contained in the file must be sparse, real, and square. If the matrix
 * is not symmetric, it will be made symmetric with (A+A')/2. If the matrix has
 * more than one connected component, the largest will be found and the rest
 * discarded. If a diagonal is present, it will be removed.
 */ 
cs *read_matrix (const std::string filename, MM_typecode &matcode);

/** 
 * @brief Generate a Graph from a Matrix Market file.
 * 
 * Generate a Graph class instance from a Matrix Market file. The matrix
 * contained in the file must be sparse, real, and square. If the matrix
 * is not symmetric, it will be made symmetric with (A+A')/2. If the matrix has
 * more than one connected component, the largest will be found and the rest
 * discarded. If a diagonal is present, it will be removed.
 */ 
Graph *read_graph (const char* filename);

/** 
 * @brief Generate a CSparse matrix from a Matrix Market file.
 * 
 * Generate a cs struct instance from a Matrix Market file. The matrix
 * contained in the file must be sparse, real, and square. If the matrix
 * is not symmetric, it will be made symmetric with (A+A')/2. If the matrix has
 * more than one connected component, the largest will be found and the rest
 * discarded. If a diagonal is present, it will be removed.
 */ 
cs *read_matrix (const char* filename, MM_typecode &matcode);

} // end namespace Mongoose

#endif