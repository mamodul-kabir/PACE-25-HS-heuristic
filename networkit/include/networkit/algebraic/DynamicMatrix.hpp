/*
 * DynamicMatrix.hpp
 *
 *  Created on: 13.03.2014
 *      Author: Michael Wegner (michael.wegner@student.kit.edu)
 */

#ifndef NETWORKIT_ALGEBRAIC_DYNAMIC_MATRIX_HPP_
#define NETWORKIT_ALGEBRAIC_DYNAMIC_MATRIX_HPP_

#include <iostream>

#include <networkit/algebraic/AlgebraicGlobals.hpp>
#include <networkit/algebraic/SparseAccumulator.hpp>
#include <networkit/algebraic/Vector.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

/**
 * @ingroup algebraic
 * The DynamicMatrix class represents a matrix that is optimized for sparse matrices and internally
 * uses a graph data structure. DynamicMatrix should be used when changes to the structure of the
 * matrix are frequent.
 */
class DynamicMatrix final {
protected:
    Graph graph;

    count nRows;
    count nCols;

    double zero;

    // necessary for operator() const to allow for index addressing via DynamicMatrix(i,j) = value
    class IndexProxy {
    public:
        IndexProxy(DynamicMatrix &mat, index i, index j) : Matrix{mat}, i{i}, j{j} {}

        operator double() const { return const_cast<const DynamicMatrix &>(Matrix)(i, j); }

        void operator=(double rhs) { Matrix.setValue(i, j, rhs); }

    private:
        DynamicMatrix &Matrix;
        index i;
        index j;
    };

public:
    /** Default constructor */
    DynamicMatrix();

    /**
     * Constructs the Matrix with size @a dimension x @a dimension.
     * @param dimension Defines how many rows and columns this matrix has.
     * @param zero The zero element (default is 0.0).
     */
    DynamicMatrix(count dimension, double zero = 0.0);

    /**
     * Constructs the Matrix with size @a nRows x @a nCols.
     * @param nRows Number of rows.
     * @param nCols Number of columns.
     * @param zero The zero element (default is 0.0).
     */
    DynamicMatrix(count nRows, count nCols, double zero = 0.0);

    /**
     * Constructs the @a dimension x @a dimension Matrix from the elements at position @a positions
     * with values @values.
     * @param dimension Defines how many rows and columns this matrix has.
     * @param triplets The nonzero elements.
     * @param zero The zero element (default is 0.0).
     */
    DynamicMatrix(count dimension, const std::vector<Triplet> &triplets, double zero = 0.0);

    /**
     * Constructs the @a nRows x @a nCols Matrix from the elements at position @a positions with
     * values @values.
     * @param nRows Defines how many rows this matrix has.
     * @param nCols Defines how many columns this matrix has.
     * @param triplets The nonzero elements.
     * @param zero The zero element (default is 0.0).
     */
    DynamicMatrix(count nRows, count nCols, const std::vector<Triplet> &triplets,
                  double zero = 0.0);

    /**
     * Compares this matrix to @a other and returns true if the shape and zero element are the same
     * as well as all entries, otherwise returns false.
     * @param other
     */
    bool operator==(const DynamicMatrix &other) const {
        bool graphsEqual = graph.numberOfNodes() == other.graph.numberOfNodes()
                           && graph.numberOfEdges() == other.graph.numberOfEdges();
        if (graphsEqual) {
            graph.forEdges([&](node u, node v, edgeweight w) {
                if (w != other.graph.weight(u, v)) {
                    graphsEqual = false;
                    return;
                }
            });
        }

        return graphsEqual && nRows == other.nRows && nCols == other.nCols && zero == other.zero;
    }

    /**
     * Compares this matrix to @a other and returns true if the shape and zero
     * element are the same as well as
     * all entries are the same (within the absolute error range of @a eps), otherwise returns
     * false.
     * @param other
     * @param eps
     */
    bool isApprox(const DynamicMatrix &other, const double eps = 0.01) const {
        bool graphsEqual = graph.numberOfNodes() == other.graph.numberOfNodes()
                           && graph.numberOfEdges() == other.graph.numberOfEdges();
        if (graphsEqual) {
            graph.forEdges([&](node u, node v, edgeweight w) {
                if (std::abs(w - other.graph.weight(u, v)) > eps) {
                    graphsEqual = false;
                    return;
                }
            });
        }

        return graphsEqual && nRows == other.nRows && nCols == other.nCols && zero == other.zero;
    }

    /**
     * Compares this matrix to @a other and returns false if the shape and zero element are the same
     * as well as all entries, otherwise returns true.
     * @param other
     */
    bool operator!=(const DynamicMatrix &other) const { return !((*this) == other); }

    /**
     * @return Number of rows.
     */
    inline count numberOfRows() const { return nRows; }

    /**
     * @return Number of columns.
     */
    inline count numberOfColumns() const { return nCols; }

    /**
     * Returns the zero element of the matrix.
     */
    inline double getZero() const { return zero; }

    /**
     * @param i The row index.
     * @return Number of non-zeros in row @a i.
     */
    count nnzInRow(index i) const;

    /**
     * @return Number of non-zeros in this matrix.
     */
    count nnz() const;

    /**
     * @return Value at matrix position (i,j).
     */
    double operator()(index i, index j) const;

    /**
     * Set the matrix at position (@a i, @a j) to @a value.
     */
    IndexProxy operator()(index i, index j) { return IndexProxy(*this, i, j); }

    /**
     * Set the matrix at position (@a i, @a j) to @a value.
     */
    void setValue(index i, index j, double value);

    /**
     * @return Row @a i of this matrix as vector.
     */
    Vector row(index i) const;

    /**
     * @return Column @a j of this matrix as vector.
     */
    Vector column(index j) const;

    /**
     * @return The main diagonal of this matrix.
     */
    Vector diagonal() const;

    /**
     * Adds this matrix to @a other and returns the result.
     * @return The sum of this matrix and @a other.
     */
    DynamicMatrix operator+(const DynamicMatrix &other) const;

    /**
     * Adds @a other to this matrix.
     * @return Reference to this matrix.
     */
    DynamicMatrix &operator+=(const DynamicMatrix &other);

    /**
     * Subtracts @a other from this matrix and returns the result.
     * @return The difference of this matrix and @a other.
     *
     */
    DynamicMatrix operator-(const DynamicMatrix &other) const;

    /**
     * Subtracts @a other from this matrix.
     * @return Reference to this matrix.
     */
    DynamicMatrix &operator-=(const DynamicMatrix &other);

    /**
     * Multiplies this matrix with a scalar specified in @a scalar and returns the result.
     * @return The result of multiplying this matrix with @a scalar.
     */
    DynamicMatrix operator*(double scalar) const;

    /**
     * Multiplies this matrix with a scalar specified in @a scalar.
     * @return Reference to this matrix.
     */
    DynamicMatrix &operator*=(double scalar);

    /**
     * Multiplies this matrix with @a vector and returns the result.
     * @return The result of multiplying this matrix with @a vector.
     */
    Vector operator*(const Vector &vector) const;

    /**
     * Multiplies this matrix with @a other and returns the result in a new matrix.
     * @return The result of multiplying this matrix with @a other.
     */
    DynamicMatrix operator*(const DynamicMatrix &other) const;

    /**
     * Divides this matrix by a divisor specified in @a divisor and returns the result in a new
     * matrix.
     * @return The result of dividing this matrix by @a divisor.
     */
    DynamicMatrix operator/(double divisor) const;

    /**
     * Divides this matrix by a divisor specified in @a divisor.
     * @return Reference to this matrix.
     */
    DynamicMatrix &operator/=(double divisor);

    /**
     * Computes A^T * B.
     * @param A
     * @param B
     */
    static DynamicMatrix mTmMultiply(const DynamicMatrix &A, const DynamicMatrix &B);

    /**
     * Computes A * B^T.
     * @param A
     * @param B
     */
    static DynamicMatrix mmTMultiply(const DynamicMatrix &A, const DynamicMatrix &B);

    /**
     * Computes matrix^T * vector
     * @param matrix
     * @param vector
     */
    static Vector mTvMultiply(const DynamicMatrix &matrix, const Vector &vector);

    /**
     * Transposes this matrix and returns it.
     */
    DynamicMatrix transpose() const;

    /**
     * Extracts a matrix with rows and columns specified by @a rowIndices and @a columnIndices from
     * this matrix. The order of rows and columns is equal to the order in @a rowIndices and @a
     * columnIndices. It is also possible to specify a row or column more than once to get
     * duplicates.
     * @param rowIndices
     * @param columnIndices
     */
    DynamicMatrix extract(const std::vector<index> &rowIndices,
                          const std::vector<index> &columnIndices) const;

    /**
     * Assign the contents of the matrix @a source to this matrix at rows and columns specified by
     * @a rowIndices and
     * @a columnIndices. That is, entry (i,j) of @a source is assigned to entry (rowIndices[i],
     * columnIndices[j]) of this matrix. Note that the dimensions of @rowIndices and @a
     * columnIndices must coincide with the number of rows and columns of @a source.
     * @param rowIndices
     * @param columnIndices
     * @param source
     */
    void assign(const std::vector<index> &rowIndices, const std::vector<index> &columnIndices,
                const DynamicMatrix &source);

    /**
     * Applies the unary function @a unaryElementFunction to each value in the matrix. Note that it
     * must hold that the function applied to the zero element of this matrix returns the zero
     * element.
     * @param unaryElementFunction
     */
    template <typename F>
    void apply(F unaryElementFunction);

    /**
     * Returns the (weighted) adjacency matrix of the (weighted) Graph @a graph.
     * @param graph
     */
    static DynamicMatrix adjacencyMatrix(const Graph &graph, double zero = 0.0);

    /**
     * Creates a diagonal matrix with dimension equal to the dimension of the Vector @a
     * diagonalElements. The values on the diagonal are the ones stored in @a diagonalElements (i.e.
     * D(i,i) = diagonalElements[i]).
     * @param diagonalElements
     */
    static DynamicMatrix diagonalMatrix(const Vector &diagonalElements, double zero = 0.0);

    /**
     * Returns the (weighted) incidence matrix of the (weighted) Graph @a graph.
     * @param graph
     */
    static DynamicMatrix incidenceMatrix(const Graph &graph, double zero = 0.0);

    /**
     * Returns the (weighted) Laplacian matrix of the (weighteD) Graph @a graph.
     * @param graph
     */
    static DynamicMatrix laplacianMatrix(const Graph &graph, double zero = 0.0);

    /**
     * Returns the (weighted) normalized Laplacian matrix of the (weighted) Graph @a graph
     * @param graph
     */
    static DynamicMatrix normalizedLaplacianMatrix(const Graph &graph, double zero = 0.0);

    /**
     * Iterate over all non-zero elements of row @a row in the matrix and call handle(index row,
     * index column, double value)
     */
    template <typename L>
    void forNonZeroElementsInRow(index row, L handle) const;

    /**
     * Iterate in parallel over all non-zero elements of row @a row in the matrix and call
     * handler(index column, double value)
     */
    template <typename L>
    void parallelForNonZeroElementsInRow(index i, L handle) const;

    /**
     * Iterate over all elements in row @a i in the matrix and call handle(index column, double
     * value)
     */
    template <typename L>
    void forElementsInRow(index i, L handle) const;

    /**
     * Iterate in parallel over all elements (including zeros) of row @a row in the matrix and call
     * handler(index column, double value)
     */
    template <typename L>
    void parallelForElementsInRow(index row, L handle) const;

    /**
     * Iterate over all elements (including zeros) of the matrix in row order and call handler
     * (lambda closure).
     */
    template <typename L>
    void forElementsInRowOrder(L handle) const;

    /**
     * Iterate in parallel over all elements (including zeros) in row order and call handle
     * (lambda closure) on elements of the matrix.
     */
    template <typename L>
    void parallelForElementsInRowOrder(L handle) const;

    /**
     * Iterate over all non-zero elements of the matrix in row order and call handle(index row,
     * index column, double value).
     */
    template <typename L>
    void forNonZeroElementsInRowOrder(L handle) const;

    /**
     * Iterate in parallel over all rows and call handle(index row, index column, double value) on
     * non-zero elements of the matrix.
     */
    template <typename L>
    void parallelForNonZeroElementsInRowOrder(L handle) const;
};

template <typename F>
void NetworKit::DynamicMatrix::apply(F unaryElementFunction) {
    forNonZeroElementsInRowOrder(
        [&](index i, index j, double value) { setValue(i, j, unaryElementFunction(value)); });
}

template <typename L>
inline void NetworKit::DynamicMatrix::forNonZeroElementsInRow(index row, L handle) const {
    graph.forEdgesOf(row, [&](index j, edgeweight weight) { handle(j, weight); });
}

template <typename L>
inline void NetworKit::DynamicMatrix::parallelForNonZeroElementsInRow(index i, L handle) const {
    std::vector<std::pair<index, edgeweight>> elements(graph.weightNeighborRange(i).begin(),
                                                       graph.weightNeighborRange(i).end());
#pragma omp parallel for
    for (omp_index j = 0; j < static_cast<omp_index>(elements.size()); ++j) {
        handle(elements[j].first, elements[j].second);
    }
}

template <typename L>
inline void NetworKit::DynamicMatrix::forElementsInRow(index i, L handle) const {
    Vector rowVector = row(i);
    index j = 0;
    rowVector.forElements([&](double value) { handle(j++, value); });
}

template <typename L>
inline void NetworKit::DynamicMatrix::parallelForElementsInRow(index i, L handle) const {
    Vector rowVector = row(i);
#pragma omp parallel for
    for (omp_index j = 0; j < static_cast<omp_index>(rowVector.getDimension()); ++j) {
        handle(j, rowVector[j]);
    }
}

template <typename L>
inline void NetworKit::DynamicMatrix::forElementsInRowOrder(L handle) const {
    for (index i = 0; i < nRows; ++i) {
        index col = 0;
        graph.forEdgesOf(i, [&](index j, edgeweight weight) {
            while (col < j) {
                handle(i, col, getZero());
                ++col;
            }
            handle(i, col, weight);
            ++col;
        });

        while (col < i) {
            handle(i, col, getZero());
            ++col;
        }
    }
}

template <typename L>
inline void NetworKit::DynamicMatrix::parallelForElementsInRowOrder(L handle) const {
#pragma omp parallel for
    for (omp_index i = 0; i < static_cast<omp_index>(nRows); ++i) {
        index col = 0;
        graph.forEdgesOf(i, [&](index j, edgeweight weight) {
            while (col < j) {
                handle(i, col, getZero());
                ++col;
            }
            handle(i, col, weight);
            ++col;
        });

        while (col < i) {
            handle(i, col, getZero());
            ++col;
        }
    }
}

template <typename L>
inline void NetworKit::DynamicMatrix::forNonZeroElementsInRowOrder(L handle) const {
    for (index i = 0; i < nRows; ++i) {
        graph.forEdgesOf(i, [&](index j, edgeweight weight) { handle(i, j, weight); });
    }
}

template <typename L>
inline void NetworKit::DynamicMatrix::parallelForNonZeroElementsInRowOrder(L handle) const {
#pragma omp parallel for
    for (omp_index i = 0; i < static_cast<omp_index>(nRows); ++i) {
        graph.forEdgesOf(i, [&](index j, edgeweight weight) { handle(i, j, weight); });
    }
}

// print functions for test debugging / output
inline std::ostream &operator<<(std::ostream &os, const DynamicMatrix &M) {
    for (index row = 0; row < M.numberOfRows(); ++row) {
        if (row != 0)
            os << std::endl;
        for (index col = 0; col < M.numberOfColumns(); ++col) {
            if (col != 0)
                os << ", ";
            os << M(row, col);
        }
    }
    return os;
}
} // namespace NetworKit

#endif // NETWORKIT_ALGEBRAIC_DYNAMIC_MATRIX_HPP_
