/*
 * GraphBuilder.hpp
 *
 *  Created on: 15.07.2014
 *      Author: Marvin Ritter (marvin.ritter@gmail.com)
 */

#ifndef NETWORKIT_GRAPH_GRAPH_BUILDER_HPP_
#define NETWORKIT_GRAPH_GRAPH_BUILDER_HPP_

#include <vector>

#include <networkit/Globals.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

/*
 * The GraphBuilder helps to speed up graph generation by minimizing the number
 * of checks on addEdge/setWeight/increaseWeight. Further more it delays the
 * construction of some internal data structures of the Graph class until you
 * call toGraph(). toGraph() can only be called once. In the Graph class for an
 * edge u -> v, v is stored in the adjacent array of u (first half) and u in the
 * adjacent array of v (second half). (For directed graphs these might be in and
 * out adjacent arrays.). So each edge can be seen as a pair of 2 half edges. To
 * allow optimization and mainly parallelization GraphBuilder lets you add both
 * half edges yourself. You are responsible for adding both half edges,
 * otherwise you might end up with an invalid Graph object. As adding the first
 * half edge of an edge u -> v only requires access to the adjacent array of u,
 * other threads can add edges a -> b as long as a != u. Some goes for the
 * methods setWeight and increaseWeight. Note: If you add the first half edge of
 * u -> v, you can change the weight by calling setWeight(u, v, ew) or
 * increaseWeight(u, v, ew), but calling setWeight(v, u, ew) or
 * increaseWeight(v, u, ew) will add the second half edge. GraphBuilder allows
 * you to be lazy and only add one half of each edge. Calling toGraph with
 * autoCompleteEdges set to true, will make each half Edge in GraphBuilder to
 * one full edge in Graph.
 *
 * So far I didn't came up with a good parallelization for toGraph, so at some
 * point I might omit the parallel parameter for toGraph.
 */

class GraphBuilder {
    count n;         //!< current number of nodes
    count selfloops; //!< currently encountered number of self loops

    bool weighted;                  //!< true if the graph will be weighted, false otherwise
    bool directed;                  //!< true if the graph will be directed, false otherwise
    bool autoCompleteEdges = false; //!< if set to true, all function calls of addHalfEdge() will
                                    //!< add both half edges, reducing the time to build

    index indexInOutEdgeArrayPerThread(node u, node v) const;

    index indexInInEdgeArrayPerThread(node u, node v) const;

    //!< only used in parallel graph building
    struct HalfEdge {
        node source;
        node destination;
        // HalfEdge(node source, node destination);
        HalfEdge(){};
        HalfEdge(node source, node destination) : source(source), destination(destination){};
    };

    std::vector<std::vector<std::vector<HalfEdge>>>
        outEdgesPerThread; //!< outgoing adjacency list for each thread, where
                           //!< outEdges[cur_thread][u%max_threads] contains v if the edge (u,v)
                           //!< exists
    std::vector<std::vector<std::vector<edgeweight>>>
        outEdgeWeightsPerThread; //!< corresponding weights (if weighted)
    std::vector<std::vector<std::vector<HalfEdge>>>
        inEdgesPerThread; //!< ingoing adjacency list (if directed) for each thread, where
                          //!< inEdges[cur_thread][u%max_threads] contains v if the edge (u,v)
                          //!< exists
    std::vector<std::vector<std::vector<edgeweight>>>
        inEdgeWeightsPerThread; //!< corresponding weights (if weighted)

public:
    /**
     * Creates a new GraphBuilder. GraphBuilder supports the basic methods needed
     * to create a new graph (addNode, addEdge, setWeight, increaseWeight). It is
     * designed to be much faster for graph creation, but the speed comes with a
     * restriction: For undirected graphs GraphBuilder will handle u->v and v->u
     * as two different edges. Keep that in mind when using setWeight and
     * increaseWeight. GraphBuilder allows parallelization in a special way. It's
     * internal data structure saves edges only at the source node. As long as
     * edges from node u are only added/changed by thread t1, every other thread
     * can modifier edges not starting in u. addNode is not threadsafe.
     * @param n Number of nodes.
     * @param weighted If set to <code>true</code>, the graph has edge weights.
     * @param directed If set to @c true, the graph will be directed.
     * @param autoCompleteEdges If set to @c true, the edges will automatically be added to the
     * adjacency lists of both nodes (decreases the time to build).
     */
    GraphBuilder(count n = 0, bool weighted = false, bool directed = false,
                 bool autoCompleteEdges = false);

    void reset(count n = 0);

    /**
     * Returns <code>true</code> if this graph supports edge weights other
     * than 1.0.
     * @return <code>true</code> if this graph supports edge weights other
     * than 1.0.
     */
    inline bool isWeighted() const { return weighted; }

    /**
     * Return <code>true</code> if this graph supports directed edges.
     * @return </code>true</code> if this graph supports directed edges.
     */
    inline bool isDirected() const { return directed; }

    /**
     * Return <code>true</code> if graph contains no nodes.
     * @return <code>true</code> if graph contains no nodes.
     */
    inline bool isEmpty() const { return n == 0; }

    /**
     * Return the number of nodes in the graph.
     * @return The number of nodes.
     */
    count numberOfNodes() const { return n; }

    /**
     * Get an upper bound for the node ids in the graph.
     * @return An upper bound for the node ids.
     */
    index upperNodeIdBound() const { return n; }

    /**
     * Add a new node to the graph and return it.
     * @return The new node.
     */
    node addNode();

    /**
     * Insert an edge between the nodes @a u and @a v. If the graph is weighted
     * you can optionally set a weight for this edge. The default weight is 1.0.
     * If setUseWholeEdges(true) has been called prior, this sets both half edges, which improves
     * the performance of the graph builder.
     * @param u Endpoint of edge.
     * @param v Endpoint of edge.
     * @param weight Optional edge weight.
     */
    void addHalfEdge(node u, node v, edgeweight ew = defaultEdgeWeight);
    void addHalfOutEdge(node u, node v, edgeweight ew = defaultEdgeWeight);
    void addHalfInEdge(node u, node v, edgeweight ew = defaultEdgeWeight);

    void swapNeighborhood(node u, std::vector<node> &neighbours, std::vector<edgeweight> &weights,
                          bool selfloop);

    /**
     * Set the weight of an edge. If the edge does not exist,
     * it will be inserted.
     *
     * @param[in]  u  endpoint of edge
     * @param[in]  v  endpoint of edge
     * @param[in]  weight  edge weight
     */
    void setWeight(node u, node v, edgeweight ew) { setOutWeight(u, v, ew); }
    void setOutWeight(node u, node v, edgeweight ew);
    void setInWeight(node u, node v, edgeweight ew);

    /**
     * Increase the weight of an edge. If the edge does not exist,
     * it will be inserted.
     *
     * @param[in]  u  endpoint of edge
     * @param[in]  v  endpoint of edge
     * @param[in]  ew  edge weight
     */
    void increaseWeight(node u, node v, edgeweight ew) { increaseOutWeight(u, v, ew); }
    void increaseOutWeight(node u, node v, edgeweight ew);
    void increaseInWeight(node u, node v, edgeweight ew);

    /**
     * Generates a Graph instance. The graph builder will be resetted at the end.
     */
    Graph completeGraph();

    /**
     * DEPRECATED: use completeGraph() instead which uses the parallel mode by default (if
     * possible).
     */
    Graph TLX_DEPRECATED(completeGraph([[maybe_unused]] bool parallel)) {
        WARN("GraphBuilder::completeGraph(bool parallel) is deprecated, use "
             "GraphBuilder::completeGraph() instead");
        return completeGraph();
    }

    /**
     * Iterate over all nodes of the graph and call @a handle (lambda closure).
     *
     * @param handle Takes parameter <code>(node)</code>.
     */
    template <typename L>
    void forNodes(L handle) const;

    /**
     * Iterate randomly over all nodes of the graph and call @a handle (lambda
     * closure).
     *
     * @param handle Takes parameter <code>(node)</code>.
     */
    template <typename L>
    void parallelForNodes(L handle) const;

    /**
     * Iterate over all undirected pairs of nodes and call @a handle (lambda
     * closure).
     *
     * @param handle Takes parameters <code>(node, node)</code>.
     */
    template <typename L>
    void forNodePairs(L handle) const;

    /**
     * Iterate over all undirected pairs of nodes in parallel and call @a handle
     * (lambda closure).
     *
     * @param handle Takes parameters <code>(node, node)</code>.
     */
    template <typename L>
    void parallelForNodePairs(L handle) const;

    /*
     * If set to true, the graph builder automatically adds the second part of the halfEdge when
     * calling <code>addHalfEdge(u,v)</code>. This enables the graph building process to be more
     * efficient, while adding the halfEdges takes slightly longer.
     */
    void setAutoCompleteEdges(bool completeEdges = false) { autoCompleteEdges = completeEdges; };

private:
    void toGraphSequential(Graph &G);
    void toGraphParallel(Graph &G);
    void addHalfEdgesToGraph(Graph &G);

    template <typename T>
    static void copyAndClear(std::vector<T> &source, std::vector<T> &target);

    count numberOfEdges(const Graph &G);
};

template <typename L>
void GraphBuilder::forNodes(L handle) const {
    for (node v = 0; v < n; v++) {
        handle(v);
    }
}

template <typename L>
void GraphBuilder::parallelForNodes(L handle) const {
#pragma omp parallel for schedule(dynamic, 100)
    for (omp_index v = 0; v < static_cast<omp_index>(n); v++) {
        handle(v);
    }
}

template <typename L>
void GraphBuilder::forNodePairs(L handle) const {
    for (node u = 0; u < n; u++) {
        for (node v = u + 1; v < n; v++) {
            handle(u, v);
        }
    }
}

template <typename L>
void GraphBuilder::parallelForNodePairs(L handle) const {
#pragma omp parallel for schedule(dynamic, 100)
    for (omp_index u = 0; u < static_cast<omp_index>(n); u++) {
        for (node v = u + 1; v < n; v++) {
            handle(u, v);
        }
    }
}

template <typename T>
void GraphBuilder::copyAndClear(std::vector<T> &source, std::vector<T> &target) {
    std::copy(source.begin(), source.end(), std::back_inserter(target));
    source.clear();
}

} /* namespace NetworKit */

#endif // NETWORKIT_GRAPH_GRAPH_BUILDER_HPP_
