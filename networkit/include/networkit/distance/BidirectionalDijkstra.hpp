/*
 * BidirectionalDijkstra.hpp
 *
 *  Created on: 14.06.2019
 *      Author: Eugenio Angriman <angrimae@hu-berlin.de>
 */

#ifndef NETWORKIT_DISTANCE_BIDIRECTIONAL_DIJKSTRA_HPP_
#define NETWORKIT_DISTANCE_BIDIRECTIONAL_DIJKSTRA_HPP_

#include <networkit/auxiliary/VectorComparator.hpp>
#include <networkit/distance/STSP.hpp>

#include <tlx/container/d_ary_addressable_int_heap.hpp>

namespace NetworKit {

/*
 * @ingroup distance
 * Bidirectional implementation of the Dijkstra algorithm from two given source and target nodes.
 * Explores the graph from both the source and target nodes until the two explorations meet.
 */
class BidirectionalDijkstra final : public STSP {

public:
    /**
     * Creates the BidirectionalDijkstra class for a graph @a G, source node @a source, and
     * target node @a target.
     *
     * @param G The graph.
     * @param source The source node.
     * @param target The target node.
     * @param storePred If true, the algorithm will also store the predecessors
     * and reconstruct a shortest path from @a source and @a target.
     */
    BidirectionalDijkstra(const Graph &G, node source, node target, bool storePred = true)
        : STSP(G, source, target, storePred) {}

    /*
     * Runs the bidirectional Dijkstra algorithm.
     */
    void run() override;

private:
    std::vector<edgeweight> dist1;
    std::vector<edgeweight> dist2;
    std::vector<node> predT;
    std::vector<uint8_t> visited;
    uint8_t ts = 0;
    static constexpr uint8_t ballMask = uint8_t(1) << 7;

    void buildPaths(std::stack<std::deque<node>> &pathStack);

    struct CompareST {
    public:
        CompareST(const std::vector<edgeweight> &d1, const std::vector<edgeweight> &d2)
            : d1(d1), d2(d2) {}
        bool operator()(node u, node v) { return d1[u] + d2[u] < d1[v] + d2[v]; }

    private:
        const std::vector<edgeweight> &d1, &d2;
    };

    tlx::d_ary_addressable_int_heap<node, 2, Aux::LessInVector<edgeweight>> h1{dist1};
    tlx::d_ary_addressable_int_heap<node, 2, Aux::LessInVector<edgeweight>> h2{dist2};
    tlx::d_ary_addressable_int_heap<node, 2, CompareST> stH{CompareST(dist1, dist2)};
};
} // namespace NetworKit
#endif // NETWORKIT_DISTANCE_BIDIRECTIONAL_DIJKSTRA_HPP_
