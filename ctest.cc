#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include "src/lru.cpp"   
#include "src/fifo.cpp" 
#include "src/clock.cpp"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <trace_file> [capacity]" << endl;
        return 1;
    }
    string traceFile = argv[1];
    int capacity = 10000; 
    if (argc >= 3)
        capacity = atoi(argv[2]);

    vector<int> trace;
    ifstream in(traceFile);
    if (!in) {
        cerr << "Cannot open file: " << traceFile << endl;
        return 1;
    }

    int col1, col2, addr;
    while (in >> col1 >> col2 >> addr) {
        trace.push_back(addr);
    }
    
    if (!in.eof()) {
        cerr << "Error reading file - invalid format" << endl;
        return 1;
    }
    in.close();

    LRU lru(capacity);
    fifo fifoSim(capacity);
    Clock clockSim(capacity);

    for (int a : trace)
        lru.access(a);
    int lruAcc, lruMiss, lruFill;
    lru.data(lruAcc, lruMiss, lruFill);
    double lruHitRate = lru.hit_rate();

    for (int a : trace)
        fifoSim.access(a);
    int fifoAcc, fifoMiss, fifoFill;
    fifoSim.data(fifoAcc, fifoMiss, fifoFill);
    double fifoHitRate = fifoSim.hit_rate();

    for (int a : trace)
        clockSim.access(a);
    auto clkStats = clockSim.data(); 
    double clockHitRate = clockSim.hitRate();

    cout << "=== Cache Simulation Statistics ===" << endl << endl;
    
    cout << "LRU:" << endl;
    cout << "  Accesses: " << lruAcc << ", Misses: " << lruMiss 
         << ", Cache fill time: " << lruFill << endl;
    cout << "  Hit rate: " << lruHitRate << endl << endl;
    
    cout << "FIFO:" << endl;
    cout << "  Accesses: " << fifoAcc << ", Misses: " << fifoMiss 
         << ", Cache fill time: " << fifoFill << endl;
    cout << "  Hit rate: " << fifoHitRate << endl << endl;
    
    cout << "CLOCK:" << endl;
    cout << "  Accesses: " << clkStats.nAcc << ", Misses: " << clkStats.nMiss 
         << ", Cache fill time: " << clkStats.nFill 
         << ", Recycles: " << clkStats.nRecycle << endl;
    cout << "  Hit rate: " << clockHitRate << endl << endl;
    
    return 0;
}
