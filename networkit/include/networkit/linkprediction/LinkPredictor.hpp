/*
 * LinkPredictor.hpp
 *
 *  Created on: 28.02.2015
 *      Author: Kolja Esders
 */

#ifndef NETWORKIT_LINKPREDICTION_LINK_PREDICTOR_HPP_
#define NETWORKIT_LINKPREDICTION_LINK_PREDICTOR_HPP_

#include <memory>

#include <networkit/graph/Graph.hpp>

namespace NetworKit {

/**
 * @ingroup linkprediction
 *
 * Abstract base class for link predictors.
 */
class LinkPredictor {
public:
    // Declare typedef in advance for use in the protected section
    using prediction = std::pair<std::pair<node, node>, double>; //!< Type of predictions

private:
    /**
     * Subclasses implement this private method to inject the custom predictor-code.
     * Subclasses don't have to check whether the arguments are valid or a graph is set.
     * @param u First node in graph
     * @param v Second node in graph
     * @return a prediction-score indicating the likelihood of a future link between the given nodes
     */
    virtual double runImpl(node u, node v) = 0;

protected:
    const Graph *G; //!< Graph to operate on

    bool validCache; //!< Indicates whether a possibly used cache is valid

public:
    LinkPredictor();

    /**
     *
     * @param G The graph to work on
     */
    explicit LinkPredictor(const Graph &G);

    /**
     * Default destructor.
     */
    virtual ~LinkPredictor() = default;

    /**
     * Sets the graph to work on.
     * @param newGraph The graph to work on
     */
    virtual void setGraph(const Graph &newGraph);

    /**
     * Returns a score indicating the likelihood of a future link between the given nodes.
     * Prior to calling this method a graph should be provided through the constructor or
     * by calling setGraph. Note that only undirected graphs are accepted.
     * There is also no lower or upper bound for scores and the actual range of values depends
     * on the specific link predictor implementation. In case @a u == @a v a 0 is returned.
     * If suitable this method might make use of parallelization to enhance performance.
     * @param u First node in graph
     * @param v Second node in graph
     * @return a prediction-score indicating the likelihood of a future link between the given nodes
     */
    virtual double run(node u, node v);

    /**
     * Executes the run-method on al given @a nodePairs and returns a vector of predictions.
     * The result is a vector of pairs where the first element is the node-pair and it's second
     * element the corresponding score generated by the run-method. The method makes use of
     * parallelization.
     * @param nodePairs Node-pairs to run the predictor on
     * @return a vector of pairs containing the given node-pair as the first element and it's
     * corresponding score as the second element. The vector is sorted ascendingly by node-pair
     */
    virtual std::vector<prediction> runOn(std::vector<std::pair<node, node>> nodePairs);

    /**
     * Runs the link predictor on all currently unconnected node-pairs.
     * Possible self-loops are also excluded. The method makes use of parallelization.
     * @return a vector of pairs containing all currently unconnected node-pairs as the first
     * elements and the corresponding scores as the second elements. The vector is sorted
     * ascendingly by node-pair
     */
    virtual std::vector<prediction> runAll();
};

} // namespace NetworKit

#endif // NETWORKIT_LINKPREDICTION_LINK_PREDICTOR_HPP_
