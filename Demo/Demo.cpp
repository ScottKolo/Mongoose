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
    clock_t trial_start;
    double duration, trial_duration;

    Options *options;

    for (int k = 0; k < 12; k++)
    {
        cout << "**************************************************" << endl;
        cout << "Computing an edge cut for " << demo_files[k] << "..." << endl;
        
        trial_start = clock();
        options = Options::Create();
        if (!options) return 1; // Return an error if we failed.

        options->doExpensiveChecks = false;
        options->matchingStrategy = HEMDavisPA;
        options->guessCutType = QP_BallOpt;

        Graph *G = readGraph("../Matrix/" + demo_files[k]);
        if (!G)
        {
            free(options);
            return 1;
        }

        ComputeEdgeSeparator (G, options);

        cout << "Partitioning Complete!" << endl;
        printf("Cut Cost:      %.2f\n", G->cutCost);
        printf("Cut Imbalance: %.2f%%\n", fabs(100*G->imbalance));

        trial_duration = (std::clock() - trial_start) / (double) CLOCKS_PER_SEC;
        printf("Trial Time:    %.0fms\n", trial_duration*1000);

        G->~Graph();
        free(G);
        free(options);
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
