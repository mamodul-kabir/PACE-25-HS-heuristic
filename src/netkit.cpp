#include "netkit.h"

using namespace std;

NetworKit::Graph G(1, false, false);
vector<NetworKit::count> coreNumbers;

void run() {
    NetworKit::count n = G.numberOfNodes();

    // Core Decomposition
    NetworKit::CoreDecomposition coreDecomp(G);
    coreDecomp.run();
    coreNumbers = coreDecomp.getPartition().getVector();
}
