/**
 * Demo.cpp
 * Runs a variety of computations on several input matrices and outputs
 * the results. Does not take any input. This application can be used to
 * test that compilation was successful and that everything is working
 * properly.
 */

#include "Mongoose_Internal.hpp"
#include "Mongoose_EdgeSeparator.hpp"
#include "Mongoose_CSparse.hpp"
#include "Mongoose_IO.hpp"
#include <ctime>
#include <string>
#include <iostream>

using namespace Mongoose;
using namespace std;

const std::string demo_files[12] = {
    "bcspwr01.mtx",
    "bcspwr02.mtx",
    "bcspwr03.mtx",
    "bcspwr04.mtx",
    "bcspwr05.mtx",
    "bcspwr06.mtx",
    "bcspwr07.mtx",
    "bcspwr08.mtx",
    "bcspwr09.mtx",
    "bcspwr10.mtx",
    "jagmesh7.mtx",
    "troll.mtx"
};

int main(int argn, const char **argv)
{
    clock_t start = clock();
    double duration;

    for (int k = 0; k < 12; k++)
    {
        cout << "**************************************************" << endl;
        cout << "Computing an edge cut for " << demo_files[k] << "..." << endl;
        
        clock_t trial_start = clock();
        Options *options = Options::Create();
        if (!options) return EXIT_FAILURE; // Return an error if we failed.

        options->matchingStrategy = HEMDavisPA;
        options->guessCutType = QP_BallOpt;

        Graph *graph = readGraph("../Matrix/" + demo_files[k]);
        if (!graph)
        {
            free(options);
            return 1;
        }

        ComputeEdgeSeparator (graph, options);

        cout << "Partitioning Complete!" << endl;
        printf("Cut Cost:      %.2f\n", graph->cutCost);
        printf("Cut Imbalance: %.2f%%\n", fabs(100*graph->imbalance));

        clock_t trial_duration = (std::clock() - trial_start) / (double) CLOCKS_PER_SEC;
        printf("Trial Time:    %.0lums\n", trial_duration*1000);

        graph->~Graph();
        SuiteSparse_free(graph);
        SuiteSparse_free(options);
    }

    duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;

    cout << "**************************************************" << endl;
    printf("Total Demo Time: %.2fs\n", duration);

    cout << endl;
    cout << "**************************************************" << endl;
    cout << "***************** Demo Complete! *****************" << endl;
    cout << "**************************************************" << endl;
    cout << endl;

    /* Return success */
    return 0;
}
