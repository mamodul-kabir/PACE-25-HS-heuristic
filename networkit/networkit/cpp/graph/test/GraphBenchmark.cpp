/*
 * GraphBenchmark.cpp
 *
 *  Created on: 01.02.2013
 *      Author: Christian Staudt (christian.staudt@kit.edu)
 */

#include <gtest/gtest.h>

#include <networkit/auxiliary/Log.hpp>
#include <networkit/auxiliary/Timer.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

class GraphBenchmark : public testing::Test {
protected:
    const int64_t n{1000};
};

// TASK: benchmark edge insertions standard vs raw

TEST_F(GraphBenchmark, edgeInsertions_noop_seq) {
    int64_t n = this->n;
    Aux::Timer runtime;

    Graph G(n);
    int64_t i = 0;
    runtime.start();
    G.forNodePairs([&](node, node) { i++; });
    runtime.stop();

    TRACE("counted i = ", i);

    INFO("[DONE] edgeInsertions_noop_seq (", runtime.elapsed().count(), " ms)");
}

TEST_F(GraphBenchmark, edgeInsertions_noop_par) {
    int64_t n = this->n;
    Aux::Timer runtime;

    Graph G(n);
    int64_t i = 0;
    runtime.start();
    G.parallelForNodePairs([&](node, node) { i++; });
    runtime.stop();

    TRACE("counted i = ", i);

    INFO("[DONE] edgeInsertions_noop_par (", runtime.elapsed().count(), " ms)");
}

TEST_F(GraphBenchmark, edgeInsertions_standard_seq) {
    count n = this->n;
    Aux::Timer runtime;

    Graph G(n);
    runtime.start();
    G.forNodePairs([&](node u, node v) { G.addEdge(u, v); });
    runtime.stop();

    INFO("[DONE] edgeInsertions_standard_seq (", runtime.elapsed().count(), " ms)");
    EXPECT_EQ((n * (n - 1)) / 2, G.numberOfEdges());
}

// Task: precompute incident weights with different methods

TEST_F(GraphBenchmark, weightedDegree_standard_seq) {
    int64_t n = this->n;
    Graph G(n);
    G.forNodePairs([&](node u, node v) { G.addEdge(u, v); });

    Aux::Timer runtime;

    runtime.start();
    std::vector<double> weightedDegree(n, 0.0);

    G.forNodes([&](node v) { weightedDegree[v] = G.weightedDegree(v); });
    runtime.stop();

    INFO("[DONE] (", runtime.elapsed().count(), " ms)");

    // test correctness of result
    bool correct = true;
    G.forNodes([&](node v) { correct &= (weightedDegree[v] == (n - 1)); });

    EXPECT_TRUE(correct);
}

// TEST: use different containers
// RESULT: NodeMap, vector and array are about equally fast

// TEST: parallelize

TEST_F(GraphBenchmark, weightedDegree_standard_par) {
    int64_t n = this->n;
    Graph G(n);
    G.forNodePairs([&](node u, node v) { G.addEdge(u, v); });

    Aux::Timer runtime;

    runtime.start();
    std::vector<double> weightedDegree(n, 0.0);

    G.parallelForNodes([&](node v) { weightedDegree[v] = G.weightedDegree(v); });
    runtime.stop();

    INFO("[DONE] (", runtime.elapsed().count(), " ms)");

    // test correctness of result
    bool correct = true;
    G.forNodes([&](node v) { correct &= (weightedDegree[v] == (n - 1)); });

    EXPECT_TRUE(correct);
}

// RESULT: significant super-linear speedup regardless of target container

} /* namespace NetworKit */
