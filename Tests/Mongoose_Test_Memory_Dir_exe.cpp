#include "Mongoose_Test.hpp"
#include "Mongoose_Internal.hpp"

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <iostream>

using namespace Mongoose;

int main(int argn, const char **argv)
{
    SuiteSparse_start();
    Logger::setDebugLevel(All);
    // Get the input file from the console input.
    const char *directory = NULL;
    if (argn == 2)
        directory = argv[1] ;
    else
    {
        printf ("Usage: Mongoose_RunAllTestsOnDir <Directory>\n") ;
        return 0 ;
    }

    DIR *dir;
    struct dirent *dirp;
    if((dir  = opendir(directory)) == NULL) {
        std::cout << "Error(" << errno << ") opening " << dir << std::endl;
        return errno;
    }

    while ((dirp = readdir(dir)) != NULL)
    {
        std::string inputFile = std::string(directory) + std::string(dirp->d_name);
        if(inputFile.length() >= 5 && inputFile.substr(inputFile.length()-3) == "mtx")
        {
            std::cout << "Running Memory Test on " << inputFile << std::endl;
            runMemoryTest(inputFile);
        }
        
    }
    closedir(dir);

    SuiteSparse_finish();
    
    return 0;
}