#include <networkit/graph/Graph.hpp>
#include <networkit/centrality/ApproxBetweenness.hpp>
#include <networkit/centrality/Betweenness.hpp>
#include <networkit/centrality/ApproxCloseness.hpp>
#include <networkit/centrality/Closeness.hpp>
#include <networkit/centrality/CoreDecomposition.hpp>
#include <networkit/Globals.hpp>
#include <networkit/centrality/LocalClusteringCoefficient.hpp>


#include <vector>


extern NetworKit::Graph G;
extern std::vector<double> btwScores, clsScores;
extern std::vector<NetworKit::count> coreNumbers;
extern double norm;
extern std::vector<double> lccScores;

void run();