/*
 * MatcherGTest.cpp
 *
 *  Created on: Feb 7, 2013
 *      Author: Henning
 */

#include <gtest/gtest.h>

#include <networkit/auxiliary/Random.hpp>
#include <networkit/generators/BarabasiAlbertGenerator.hpp>
#include <networkit/generators/ErdosRenyiGenerator.hpp>
#include <networkit/graph/Graph.hpp>
#include <networkit/graph/GraphTools.hpp>
#include <networkit/io/DibapGraphReader.hpp>
#include <networkit/io/METISGraphReader.hpp>
#include <networkit/matching/BMatcher.hpp>
#include <networkit/matching/BMatching.hpp>
#include <networkit/matching/BSuitorMatcher.hpp>
#include <networkit/matching/DynamicBSuitorMatcher.hpp>
#include <networkit/matching/LocalMaxMatcher.hpp>
#include <networkit/matching/Matcher.hpp>
#include <networkit/matching/Matching.hpp>
#include <networkit/matching/PathGrowingMatcher.hpp>
#include <networkit/matching/SuitorMatcher.hpp>

namespace NetworKit {

class MatcherGTest : public testing::Test {
protected:
    bool hasUnmatchedNeighbors(const Graph &G, const BMatching &M) {
        for (const auto e : G.edgeRange())
            if (M.isUnmatched(e.u) && M.isUnmatched(e.v))
                return true;
        return false;
    }

    bool hasUnmatchedNeighbors(const Graph &G, const Matching &M) {
        for (const auto e : G.edgeRange())
            if (!M.isMatched(e.u) && !M.isMatched(e.v))
                return true;
        return false;
    }

    edgeweight computeStaticBMatchingWeight(BSuitorMatcher &staticBMatcher) {
        staticBMatcher.buildBMatching();
        const auto sm = staticBMatcher.getBMatching();
        auto res = sm.getMatches();
        return sm.weight();
    }

    edgeweight computeDynamicBMatchingWeight(DynamicBSuitorMatcher &dynBMatcher) {
        dynBMatcher.buildBMatching();
        const auto dm = dynBMatcher.getBMatching();
        auto dres = dm.getMatches();
        return dm.weight();
    }
};

TEST_F(MatcherGTest, testLocalMaxMatching) {
    {
        Graph G(10, true, true);
        EXPECT_THROW(LocalMaxMatcher{G}, std::runtime_error);
    }

    count n = 50;
    Graph G(n);
    G.forNodePairs([&](node u, node v) { G.addEdge(u, v); });

    LocalMaxMatcher localMaxMatcher(G);

    TRACE("Start localMax matching");
    localMaxMatcher.run();
    Matching M = localMaxMatcher.getMatching();
    TRACE("Finished localMax matching");

    count numExpEdges = n / 2;
    bool isProper = M.isProper(G);
    EXPECT_TRUE(isProper);
    EXPECT_EQ(M.size(G), numExpEdges);

#if !defined _WIN32 && !defined _WIN64 && !defined WIN32 && !defined WIN64
    DibapGraphReader reader;
    Graph airfoil1 = reader.read("input/airfoil1.gi");
    LocalMaxMatcher lmm(airfoil1);
    lmm.run();
    M = lmm.getMatching();
    isProper = M.isProper(airfoil1);
    EXPECT_TRUE(isProper);
    DEBUG("LocalMax on airfoil1 produces matching of size: ", M.size(G));
#endif
}

TEST_F(MatcherGTest, testLocalMaxMatchingDirectedWarning) {
    Graph G(2, false, true);
    G.addEdge(0, 1);
    EXPECT_THROW(LocalMaxMatcher localMaxMatcher(G), std::runtime_error);
}

TEST_F(MatcherGTest, testPgaMatchingOnWeightedGraph) {
    count n = 50;
    Graph G(n);
    G.forNodePairs([&](node u, node v) { G.addEdge(u, v, Aux::Random::real()); });
    PathGrowingMatcher pgaMatcher(G);
    EXPECT_NO_THROW(pgaMatcher.run());
}

TEST_F(MatcherGTest, testPgaMatchingWithSelfLoops) {
    count n = 50;
    Graph G(n);
    G.forNodePairs([&](node u, node v) { G.addEdge(u, v, Aux::Random::real()); });
    G.forNodes([&](node u) { G.addEdge(u, u); });
    EXPECT_THROW(PathGrowingMatcher pgaMatcher(G), std::invalid_argument);
}

TEST_F(MatcherGTest, testPgaMatching) {
    count n = 50;
    Graph G(n);
    G.forNodePairs([&](node u, node v) { G.addEdge(u, v); });
    PathGrowingMatcher pgaMatcher(G);

    DEBUG("Start PGA matching on 50-clique");

    pgaMatcher.run();
    Matching M = pgaMatcher.getMatching();

    count numExpEdges = n / 2;
    bool isProper = M.isProper(G);
    EXPECT_TRUE(isProper);
    EXPECT_EQ(M.size(G), numExpEdges);
    DEBUG("Finished PGA matching on 50-clique");

#if !defined _WIN32 && !defined _WIN64 && !defined WIN32 && !defined WIN64
    DibapGraphReader reader;
    Graph airfoil1 = reader.read("input/airfoil1.gi");
    PathGrowingMatcher pga2(airfoil1);
    pga2.run();
    M = pga2.getMatching();
    isProper = M.isProper(airfoil1);
    EXPECT_TRUE(isProper);
    DEBUG("PGA on airfoil1 produces matching of size: ", M.size(G));
#endif
}

TEST_F(MatcherGTest, testValidMatching) {
    auto G = METISGraphReader{}.read("input/lesmis.graph");
    G.removeSelfLoops();
    G.removeMultiEdges();

    LocalMaxMatcher pmatcher(G);
    pmatcher.run();
    Matching M = pmatcher.getMatching();

    bool isProper = M.isProper(G);
    EXPECT_TRUE(isProper);
}

TEST_F(MatcherGTest, testSuitorMatcher) {
    { // Directed graphs are not supported
        Graph G(10, true, true);
        EXPECT_THROW(SuitorMatcher{G}, std::runtime_error);
    }

    { // Graphs with self loops are not supported
        Graph G(10);
        G.addEdge(0, 0);
        G.addEdge(0, 0);
        EXPECT_THROW(SuitorMatcher{G}, std::runtime_error);
    }

    const edgeweight maxWeight = 10;

    const auto doTest = [&, maxWeight](Graph &G) -> void {
        // Test suitor matcher
        SuitorMatcher sm(G, false, true);
        sm.run();
        const auto M1 = sm.getMatching();
        EXPECT_TRUE(M1.isProper(G));
        EXPECT_FALSE(hasUnmatchedNeighbors(G, M1));

        GraphTools::sortEdgesByWeight(G, true);
        {
            auto G1 = G;
            G1.addEdge(0, 1, maxWeight);
            G1.addEdge(0, 2, maxWeight);
            EXPECT_THROW(SuitorMatcher(G1, true, true), std::runtime_error);
        }

        // Test sort suitor matcher
        SuitorMatcher ssm(G, true, true);
        ssm.run();
        const auto M2 = ssm.getMatching();
        EXPECT_TRUE(M2.isProper(G));
        EXPECT_FALSE(hasUnmatchedNeighbors(G, M2));

        // Matchings must be the same
        G.forNodes([&M1, &M2](node u) { EXPECT_EQ(M1.mate(u), M2.mate(u)); });
    };

    for (int seed : {1, 2, 3}) {
        Aux::Random::setSeed(seed, true);

        // Test unweighted
        auto G = METISGraphReader{}.read("input/PGPgiantcompo.graph");
        G.removeSelfLoops();
        G.removeMultiEdges();
        doTest(G);

        // Test weighted
        G = GraphTools::toWeighted(G);
        G.forEdges(
            [&G, maxWeight](node u, node v) { G.setWeight(u, v, Aux::Random::real(maxWeight)); });
        doTest(G);
    }
}

TEST_F(MatcherGTest, testBSuitorMatcherInvalidGraphDirected) {
    Graph G(10, true, true);
    EXPECT_THROW(BSuitorMatcher(G, 2), std::runtime_error);
}

TEST_F(MatcherGTest, testBSuitorMatcherInvalidGraphSelfLoops) {
    Graph G(10);
    G.addEdge(0, 0);
    G.addEdge(0, 0);
    EXPECT_THROW(BSuitorMatcher(G, 2), std::runtime_error);
}

TEST_F(MatcherGTest, testBSuitorMatcherInvalidGraphHoles) {
    Graph G(10);
    G = GraphTools::toWeighted(G);
    node u = GraphTools::randomNode(G);
    G.removeNode(u);
    EXPECT_THROW(BSuitorMatcher(G, 2), std::runtime_error);
    G.restoreNode(u);
    EXPECT_NO_THROW(BSuitorMatcher(G, 2));
}

TEST_F(MatcherGTest, testBSuitorMatcherTieBreaking) {
    auto G = METISGraphReader{}.read("input/tie.graph");
    G.removeSelfLoops();
    G.removeMultiEdges();

    BSuitorMatcher bsm(G, 4);
    bsm.run();
    const auto M = bsm.getBMatching();

    EXPECT_TRUE(M.isProper());
    EXPECT_FALSE(hasUnmatchedNeighbors(G, M));
}

TEST_F(MatcherGTest, testBSuitorMatcherEqualsSuitorMatcher) {
    auto G = METISGraphReader{}.read("input/lesmis.graph");
    G.removeSelfLoops();
    G.removeMultiEdges();

    SuitorMatcher sm(G, false, false);
    sm.run();
    const auto M = sm.getMatching();

    BSuitorMatcher bsm(G, 1);
    bsm.run();
    const auto bM = bsm.getBMatching();

    EXPECT_TRUE(bM.isProper());
    EXPECT_TRUE(M.isProper(G));
    EXPECT_FALSE(hasUnmatchedNeighbors(G, M));
    EXPECT_FALSE(hasUnmatchedNeighbors(G, bM));
}

TEST_F(MatcherGTest, testBSuitorMatcherConstantB) {
    for (int b : {2, 3, 4, 5}) {
        auto G = METISGraphReader{}.read("input/lesmis.graph");
        G.removeSelfLoops();
        G.removeMultiEdges();
        BSuitorMatcher bsm(G, b);
        bsm.run();
        const auto M = bsm.getBMatching();
        EXPECT_TRUE(M.isProper());
        EXPECT_FALSE(hasUnmatchedNeighbors(G, M));
    }
}

TEST_F(MatcherGTest, testBSuitorMatcherDifferentB) {
    Aux::Random::setSeed(1, true);

    auto G = METISGraphReader{}.read("input/lesmis.graph");
    G.removeSelfLoops();
    G.removeMultiEdges();
    std::vector<count> b;
    for (count i = 0; i < G.numberOfNodes(); i++) {
        b.emplace_back(Aux::Random::integer(1, (G.numberOfNodes() - 1)));
    }

    BSuitorMatcher bsm(G, b);
    bsm.run();
    const auto M = bsm.getBMatching();
    EXPECT_TRUE(M.isProper());
    EXPECT_FALSE(hasUnmatchedNeighbors(G, M));
}

TEST_F(MatcherGTest, testDynBSuitorInsertEdges) {
    METISGraphReader graphReader;
    auto G = graphReader.read("input/lesmis.graph");
    std::vector<GraphEvent> events;
    G.forEdges([&](node u, node v, edgeweight w) {
        events.emplace_back(GraphEvent{GraphEvent::EDGE_ADDITION, u, v, w});
    });
    G.removeAllEdges();

    const count b = 6;
    DynamicBSuitorMatcher dbsm(G, b);
    dbsm.run();

    for (auto &e : events) {
        G.addEdge(e.u, e.v, e.w);
        dbsm.update(e);
        BSuitorMatcher bsm(G, b);
        bsm.run();
        EXPECT_EQ(computeDynamicBMatchingWeight(dbsm), computeStaticBMatchingWeight(bsm));
    }
}

TEST_F(MatcherGTest, testDynBSuitorRemoveEdges) {
    METISGraphReader graphReader;
    auto G = graphReader.read("input/lesmis.graph");

    const count b = 6;
    DynamicBSuitorMatcher dbsm(G, b);
    dbsm.run();

    std::vector<GraphEvent> events;
    G.forEdges([&](node u, node v) {
        events.emplace_back(GraphEvent{GraphEvent::EDGE_REMOVAL, u, v});
    });

    for (auto &e : events) {
        G.removeEdge(e.u, e.v);
        dbsm.update(e);
        BSuitorMatcher bsm(G, b);
        bsm.run();
        EXPECT_EQ(computeDynamicBMatchingWeight(dbsm), computeStaticBMatchingWeight(bsm));
    }
}

TEST_F(MatcherGTest, testDynBSuitorMixedBatch) {
    METISGraphReader graphReader;
    for (int i = 0; i < 10; i++) {
        auto G = graphReader.read("input/lesmis.graph");
        const count b = 6;
        DynamicBSuitorMatcher dbsm(G, b);
        dbsm.run();

        std::vector<GraphEvent> events;
        count m = 10;

        for (count j = 0; j < m; j++) {
            uint64_t guesser = Aux::Random::integer(0, 1);
            if (guesser) {
                auto potNonEdge = GraphTools::randomNodes(G, 2);
                while (G.hasEdge(potNonEdge[0], potNonEdge[1])) {
                    potNonEdge = GraphTools::randomNodes(G, 2);
                }
                events.emplace_back(
                    GraphEvent{GraphEvent::EDGE_ADDITION, potNonEdge[0], potNonEdge[1], 1.0});
                G.addEdge(potNonEdge[0], potNonEdge[1]);
            } else {
                const auto [u, v] = GraphTools::randomEdge(G);
                events.emplace_back(GraphEvent{GraphEvent::EDGE_REMOVAL, u, v});
                G.removeEdge(u, v);
            }
        }

        dbsm.updateBatch(events);
        BSuitorMatcher bsm(G, b);
        bsm.run();
        EXPECT_EQ(computeDynamicBMatchingWeight(dbsm), computeStaticBMatchingWeight(bsm));
    }
}

} // namespace NetworKit
