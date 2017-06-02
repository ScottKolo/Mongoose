#include "Mongoose_Test.hpp"
#include "Mongoose_IO.hpp"

using namespace Mongoose;

#undef LOG_ERROR
#undef LOG_WARN
#undef LOG_INFO
#undef LOG_TEST
#define LOG_ERROR 1
#define LOG_WARN 1
#define LOG_INFO 0
#define LOG_TEST 1

int main(int argn, char** argv)
{
    (void)argn; // Unused variable
    (void)argv; // Unused variable

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
    cs *M = readMatrix(filename, matcode);
    assert(M != NULL);

    SuiteSparse_finish();

    return 0;
}