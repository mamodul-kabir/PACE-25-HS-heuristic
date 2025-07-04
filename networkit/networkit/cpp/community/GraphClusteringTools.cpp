#include <cmath>
#include <networkit/auxiliary/Log.hpp>
#include <networkit/community/GraphClusteringTools.hpp>

namespace NetworKit {

namespace GraphClusteringTools {

namespace {
float computeMaxClusterSize(const Partition &zeta) {
    std::vector<count> clusterSizes = zeta.subsetSizes();
    return (float)*std::max_element(clusterSizes.begin(), clusterSizes.end());
}
} // namespace

float getImbalance(const Partition &zeta) {
    // TODO number of nodes and not number of elements
    float avg = std::ceil((float)zeta.numberOfElements() / (float)zeta.numberOfSubsets());
    return computeMaxClusterSize(zeta) / avg;
}

float getImbalance(const Partition &zeta, const Graph &graph) {
    float avg = std::ceil((float)graph.numberOfNodes() / (float)zeta.numberOfSubsets());
    return computeMaxClusterSize(zeta) / avg;
}

Graph communicationGraph(const Graph &graph, Partition &zeta) {
    zeta.compact();
    count n = zeta.numberOfSubsets();
    Graph commGraph(n, true);

    if (graph.isWeighted()) {
        DEBUG("weighted");

        graph.forEdges([&](node u, node v, edgeweight w) {
            if (zeta[u] != zeta[v]) {
                commGraph.increaseWeight(zeta[u], zeta[v], w);
                TRACE("increase weight of ", zeta[u], " and ", zeta[v], " by ", w);
            }
        });
    } else {
        DEBUG("not weighted");

        graph.forEdges([&](node u, node v) {
            if (zeta[u] != zeta[v]) {
                commGraph.increaseWeight(zeta[u], zeta[v], 1);
                TRACE("increase weight of ", zeta[u], " and ", zeta[v], " by 1");
            }
        });
    }

    return commGraph;
}

edgeweight weightedDegreeWithCluster(const Graph &graph, const Partition &zeta, node u, index cid) {
    edgeweight wdeg = 0;

    if (graph.isWeighted()) {
        graph.forEdgesOf(u, [&](node, node v, edgeweight w) {
            if (zeta[v] == cid) {
                wdeg += w;
            }
        });
    } else {
        graph.forEdgesOf(u, [&](node, node v) {
            if (zeta[v] == cid) {
                wdeg += 1;
            }
        });
    }
    return wdeg;
}

bool isProperClustering(const Graph &G, const Partition &zeta) {
    // test whether each node has been assigned to a cluster
    bool success = true;
    G.forNodes([&](node v) {
        bool contained = zeta.contains(v);
        if (!contained) {
            ERROR("Clustering does not contain node ", v);
            success = false;
        }
    });
    return success;
}

bool isOneClustering(const Graph &, const Partition &zeta) {
    return (zeta.numberOfSubsets() == 1);
    /*	index one = data[0];	// first subset id should be equal to all others
        // TODO: use iterator forEntries and pair-wise comparison?
        for (index e = 0; e < this->z; ++e) { // FIXME constructor initializes data with z+1, so <=
       is necessary. if (data[e] != one) { return false;
            }
        }
        return true;*/
}

bool isSingletonClustering(const Graph &G, const Partition &zeta) {
    return (zeta.numberOfSubsets() == G.numberOfNodes());
}

bool equalClusterings(const Partition &zeta, const Partition &eta, Graph &G) {
    bool eq = true;
    G.parallelForEdges([&](node u, node v) {
        if (zeta.inSameSubset(u, v)) {
            if (!eta.inSameSubset(u, v)) {
                eq = false;
            }
        } else {
            if (eta.inSameSubset(u, v)) {
                eq = false;
            }
        }
    });
    return eq;
}

} // namespace GraphClusteringTools

} // namespace NetworKit
