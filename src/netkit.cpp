#include "netkit.h"

using namespace std; 

NetworKit::Graph G(1, false, false); 
vector<double> btwScores;
vector<double> clsScores;
vector<NetworKit::count> coreNumbers;
vector<double> lccScores;

void run(){
    NetworKit::count n = G.numberOfNodes();
    if (n < 100) {
        // Exact Betweenness
        NetworKit::Betweenness betweenness(G, true);
        betweenness.run();
        btwScores = betweenness.scores();
        // Exact Closeness
        NetworKit::Closeness closeness(G);
        closeness.run();
        clsScores = closeness.scores();

    } else {
        // Approximate Betweenness
        double epsilon = 0.25; // desired additive error
        double delta = 0.4;    // acceptable failure probability
        double universalConstant = 0.5;

        NetworKit::ApproxBetweenness betweenness(G, epsilon, delta, universalConstant);
        betweenness.run();
        btwScores = betweenness.scores();

        // Approximate Closeness
        int nSamples = 100;
        NetworKit::ApproxCloseness closeness(G, nSamples, 0.2, true);
        closeness.run();
        clsScores = closeness.scores();
    }

    // Core decomposition is usually fast, run always exact
    NetworKit::CoreDecomposition coreDecomp(G);
    coreDecomp.run();
    coreNumbers = coreDecomp.getPartition().getVector();

    return;
}

