// Project Identifier: 19034C8F3B1196BF8E0C6E1C0F973D2FD550B88F

#include <vector>
#include <queue>
#include <deque>
#include <getopt.h>
#include <iostream>
#include <iomanip>
#include "mineMap.h"

using namespace std;

int main (int argc, char* argv[]) {
    //getoptlong time
    ios_base::sync_with_stdio(false);
    cout << std::fixed << std::setprecision(2);

    //cout << "checkpoint 1: beginning \n";

    static struct option long_options[] = {
    {"help",        no_argument,        NULL,  'h'},
    {"verbose",     no_argument,        NULL,  'v'},
    {"median",      no_argument,        NULL,  'm'},
    {"statistics",  required_argument,  0,     's'},
    {0,             0,                  NULL,  0, }
    };
    int option_index = 0;

    //variables for input processing
    bool verbose = false;
    bool median = false;
    bool statistics = false;
    uint32_t N = 0;

    int c = getopt_long(argc, argv, "hvms:", long_options, &option_index);
    while (c != -1) {
        switch(c) {
            case 'h' :
                cout << "help message";
                return 0;
                break;
            case 'v' :
                verbose = true;
                break;
            case 'm' :
                median = true;
                break;
            case 's' :
                statistics = true;
                N = uint32_t(stoi(optarg));
                //cout << "stats selected, N: " << N << endl;
                break;
        }
        c = getopt_long(argc, argv, "hvms:", long_options, &option_index);
    }

    //cout << "checkpoint 2: command line processed\n";

    //cout << "checkpoint 3: mode " << mode << " selected.\n";

    Mine mmap = Mine(verbose, statistics, median, N); //M MODE ONLY, R NEEDS IMPLEMENTATION

    if (median) {
        //cerr << "median mode not yet implemented\n";
    }
    /* //cin the rest of the parameters according to mode
    if (!rand) {
        mmap = Mine(cin);
    } else {
        mmap = Mine();
    } */

    //cout << "checkpoint 4: map constructed, about to enter while loop\n";
    //uint32_t round = 0;

    //start processing
    while (!mmap.lost()) {
        //cout << "round: " << ++round << endl;
        bool q = mmap.investigate();
        //cout << "investigation " << round << " complete.\n";
        switch(q) {
            case (true):
                cout << "Cleared " << mmap.numTiles << " tiles containing " << mmap.numRubble << " rubble and escaped.\n";
                if (statistics) {
                    mmap.statsOut();
                    
                } 
                return 0;
                break;
            case (false):
                cerr << "run failed (empty pq)\n";
                return 1;
                break;
            default:
                break;
        }
    }
}
