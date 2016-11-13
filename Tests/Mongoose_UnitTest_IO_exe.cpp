#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"
#include "Mongoose_IO.hpp"
#include <string>
#include <stdlib.h>

using namespace Mongoose;

int main(int argn, char** argv)
{
    SuiteSparse_start();

    // Set Logger to report all messages and turn off timing info
    Logger::setDebugLevel(All);
    Logger::setTimingFlag(false);

    Graph *G;

    // Nonexistent file
    G = readGraph("../Tests/Matrix/no_such_file.mtx");        
    assert (G == NULL);
    SuiteSparse_free(G);

    // Bad header 
    G = readGraph("../Tests/Matrix/bad_header.mtx");        
    assert (G == NULL);
    SuiteSparse_free(G);
    
    // Bad matrix type
    G = readGraph("../Tests/Matrix/bad_matrix_type.mtx");
    assert (G == NULL);
    SuiteSparse_free(G);

    // Bad dimensions
    G = readGraph("../Tests/Matrix/bad_dimensions.mtx");
    assert (G == NULL);
    SuiteSparse_free(G);
      
    // Rectangular matrix     
    G = readGraph("../Tests/Matrix/Trec4.mtx");
    assert (G == NULL);
    SuiteSparse_free(G);

    // C-style string filename
    MM_typecode matcode;
    std::string filename = "../Matrix/bcspwr01.mtx";
    cs *M = readMatrix(filename.c_str(), matcode);
    assert(M != NULL);

    SuiteSparse_finish();

    return 0;
}