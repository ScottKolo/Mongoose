#include <string>
#include "mongoose_io.hpp"
#include "mongoose_conditioning.hpp"
#include <cassert>
#include "mongoose_test.hpp"
#include <iostream>
#include <fstream>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */

using namespace Mongoose;

typedef struct mm_file_data
{
    const std::string filename;
    const Int n;
    const Int nz;
} mm_file;

mm_file mm_data[] = {
    {"Pd",       1337,  2982},
    {"bcspwr01",   39,    92},
    {"bcspwr02",   49,   118},
    {"bcspwr03",  118,   358},
    {"bcspwr04",  274,  1338},
    {"bcspwr05",  443,  1180},
    {"bcspwr06", 1454,  3846},
    {"bcspwr07", 1612,  4212},
    {"bcspwr08", 1624,  4426},
    {"bcspwr09", 1723,  4788},
    {"bcspwr10", 5300, 16542}
};

void run_timing_and_results_tests()
{
    SuiteSparse_start();

    for (int k = 0; k < 11; k++)
    {
        // Given a symmetric matrix
        Options *options;
        Graph *G;
        clock_t t;
        
        options = Options::Create();
        G = read_graph("../Matrix/" + mm_data[k].filename + ".mtx");

        if (!G || !options)
        {
            // Ran out of memory
            SuiteSparse_free(options);
            G->~Graph();
            SuiteSparse_free(G);
        }
        else
        {
            // An edge separator should be computed with default options
            t = clock();
            int error = ComputeEdgeSeparator(G, options);
            t = clock() - t;

            if (error)
            {
                // Error occurred
            }
            else
            {
                std::ofstream ofs (mm_data[k].filename + "_result.txt", std::ofstream::out);
                ofs << "InputFile: " << mm_data[k].filename << ".mtx" << std::endl;
                ofs << "TotalTime: " << ((double) t)/CLOCKS_PER_SEC << std::endl;
                ofs << "CutSize: " << G->cutCost << std::endl;
                ofs << "Imbalance: " << G->imbalance << std::endl;

                for (int i = 0; i < G->n; i++)
                {
                    ofs << i << " ";
                    if (G->partition[i] == 0)
                    {
                        ofs << "A" << std::endl;
                    }
                    else
                    {
                        ofs << "B" << std::endl;
                    }
                }
                ofs.close();

                
                std::ifstream ifs ("../Tests/Results/" + mm_data[k].filename + "_result.txt");
                std::string input;

                // Read file name
                ifs.ignore(200, ' ');
                ifs >> input;
                std::cout << "File Name: " << input << std::endl;

                // Read Total Time
                ifs.ignore(200, ' ');
                ifs >> input;
                std::cout << "Total Time: " << input << std::endl;
                double ref_time = std::stod(input);
                assert(((double) t)/CLOCKS_PER_SEC <= 2*ref_time);

                // Read Cut Size
                ifs.ignore(200, ' ');
                ifs >> input;
                std::cout << "Cut size: " << input << std::endl;
                double ref_cut_size = std::stod(input);
                assert(fabs(G->cutCost) <= 1.1*fabs(ref_cut_size));

                ifs.close();

            }
        }

        G->~Graph();
        SuiteSparse_free(G);
        SuiteSparse_free(options);
    }

    SuiteSparse_finish();
}