/*
 * QuadNode.hpp
 *
 *  Created on: 21.05.2014
 *      Author: Moritz v. Looz
 */

#ifndef NETWORKIT_GENERATORS_QUADTREE_QUAD_NODE_HPP_
#define NETWORKIT_GENERATORS_QUADTREE_QUAD_NODE_HPP_

#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>

#include <networkit/auxiliary/Log.hpp>
#include <networkit/auxiliary/Parallel.hpp>
#include <networkit/geometric/HyperbolicSpace.hpp>

using std::cos;
using std::max;
using std::min;
using std::vector;

namespace NetworKit {

template <class T, bool poincare = true>
class QuadNode final {
    friend class QuadTreeGTest;

    double leftAngle;
    double minR;
    double rightAngle;
    double maxR;
    Point2DWithIndex<double> a, b, c, d;
    unsigned capacity;
    static constexpr unsigned coarsenLimit = 4;
    count subTreeSize;
    std::vector<T> content;
    std::vector<Point2DWithIndex<double>> positions;
    std::vector<double> angles;
    std::vector<double> radii;
    bool isLeaf;
    bool splitTheoretical;
    double alpha;
    double balance;
    index ID;
    double lowerBoundR;

public:
    std::vector<QuadNode> children;

    QuadNode() {
        // This should never be called.
        leftAngle = 0;
        rightAngle = 0;
        minR = 0;
        maxR = 0;
        capacity = 20;
        isLeaf = true;
        subTreeSize = 0;
        balance = 0.5;
        splitTheoretical = false;
        alpha = 1;
        lowerBoundR = maxR;
        ID = 0;
    }

    /**
     * Construct a QuadNode for polar coordinates.
     *
     * @param leftAngle Minimal angular coordinate of region, in radians from 0 to 2\pi
     * @param minR Minimal radial coordinate of region, between 0 and 1
     * @param rightAngle Maximal angular coordinate of region, in radians from 0 to 2\pi
     * @param maxR Maximal radial coordinate of region, between 0 and 1
     * @param capacity Number of points a leaf cell can store before splitting
     * @param minDiameter Minimal diameter of a quadtree node. If the node is already smaller, don't
     * split even if over capacity. Default is 0
     * @param splitTheoretical Whether to split in a theoretically optimal way or in a way to
     * decrease measured running times
     * @param alpha dispersion Parameter of the point distribution. Only has an effect if
     * theoretical split is true
     * @param diagnostics Count how many necessary and unnecessary comparisons happen in leaf cells?
     * Will cause race condition and false sharing in parallel use
     *
     */
    QuadNode(double leftAngle, double minR, double rightAngle, double maxR,
             unsigned capacity = 1000, bool splitTheoretical = false, double alpha = 1,
             double balance = 0.5) {
        if (balance <= 0 || balance >= 1)
            throw std::runtime_error("Quadtree balance parameter must be between 0 and 1.");
        if (poincare && maxR > 1)
            throw std::runtime_error(
                "The Poincare disk has a radius of 1, cannot create quadtree larger than that!");
        this->leftAngle = leftAngle;
        this->minR = minR;
        this->maxR = maxR;
        this->rightAngle = rightAngle;
        this->a = HyperbolicSpace::polarToCartesian(leftAngle, minR);
        this->b = HyperbolicSpace::polarToCartesian(rightAngle, minR);
        this->c = HyperbolicSpace::polarToCartesian(rightAngle, maxR);
        this->d = HyperbolicSpace::polarToCartesian(leftAngle, maxR);
        this->capacity = capacity;
        this->alpha = alpha;
        this->splitTheoretical = splitTheoretical;
        this->balance = balance;
        this->lowerBoundR = maxR;
        this->ID = 0;
        isLeaf = true;
        subTreeSize = 0;
    }

    void split() {
        assert(isLeaf);
        // heavy lifting: split up!
        double middleAngle = (rightAngle - leftAngle) / 2 + leftAngle;
        /**
         * we want to make sure the space is evenly divided to obtain a balanced tree
         * Simply halving the radius will cause a larger space for the outer Quadnode, resulting in
         * an unbalanced tree
         */

        double middleR;

        if (poincare) {
            if (splitTheoretical) {
                double hyperbolicOuter = HyperbolicSpace::EuclideanRadiusToHyperbolic(maxR);
                double hyperbolicInner = HyperbolicSpace::EuclideanRadiusToHyperbolic(minR);
                double hyperbolicMiddle =
                    std::acosh((1 - balance) * std::cosh(alpha * hyperbolicOuter)
                               + balance * std::cosh(alpha * hyperbolicInner))
                    / alpha;
                middleR = HyperbolicSpace::hyperbolicRadiusToEuclidean(hyperbolicMiddle);
            } else {
                double nom = maxR - minR;
                double denom = std::pow((1 - maxR * maxR) / (1 - minR * minR), 0.5) + 1;
                middleR = nom / denom + minR;
            }
        } else {
            middleR = std::acosh((1 - balance) * std::cosh(alpha * maxR)
                                 + balance * std::cosh(alpha * minR))
                      / alpha;
        }

        assert(middleR < maxR);
        assert(middleR > minR);

        QuadNode<index, poincare> southwest(leftAngle, minR, middleAngle, middleR, capacity,
                                            splitTheoretical, alpha, balance);
        QuadNode<index, poincare> southeast(middleAngle, minR, rightAngle, middleR, capacity,
                                            splitTheoretical, alpha, balance);
        QuadNode<index, poincare> northwest(leftAngle, middleR, middleAngle, maxR, capacity,
                                            splitTheoretical, alpha, balance);
        QuadNode<index, poincare> northeast(middleAngle, middleR, rightAngle, maxR, capacity,
                                            splitTheoretical, alpha, balance);
        children = {southwest, southeast, northwest, northeast};
        isLeaf = false;
    }

    /**
     * Add a point at polar coordinates (angle, R) with content input. May split node if capacity is
     * full
     *
     * @param input arbitrary content, in our case an index
     * @param angle angular coordinate of point, between 0 and 2 pi.
     * @param R radial coordinate of point, between 0 and 1.
     */
    void addContent(T input, double angle, double R) {
        assert(this->responsible(angle, R));
        if (lowerBoundR > R)
            lowerBoundR = R;
        if (isLeaf) {
            if (content.size() + 1 < capacity) {
                content.push_back(input);
                angles.push_back(angle);
                radii.push_back(R);
                Point2DWithIndex<double> pos = HyperbolicSpace::polarToCartesian(angle, R);
                positions.push_back(pos);
            } else {

                split();

                for (index i = 0; i < content.size(); i++) {
                    this->addContent(content[i], angles[i], radii[i]);
                }
                assert(subTreeSize == content.size()); // we have added everything twice
                subTreeSize = content.size();
                content.clear();
                angles.clear();
                radii.clear();
                positions.clear();
                this->addContent(input, angle, R);
            }
        } else {
            assert(!children.empty());
            for (index i = 0; i < children.size(); i++) {
                if (children[i].responsible(angle, R)) {
                    children[i].addContent(input, angle, R);
                    break;
                }
            }
            subTreeSize++;
        }
    }

    /**
     * Remove content at polar coordinates (angle, R). May cause coarsening of the quadtree
     *
     * @param input Content to be removed
     * @param angle Angular coordinate
     * @param R Radial coordinate
     *
     * @return True if content was found and removed, false otherwise
     */
    bool removeContent(T input, double angle, double R) {
        if (!responsible(angle, R))
            return false;
        if (isLeaf) {
            index i = 0;
            for (; i < content.size(); i++) {
                if (content[i] == input)
                    break;
            }
            if (i < content.size()) {
                assert(angles[i] == angle);
                assert(radii[i] == R);
                // remove element
                content.erase(content.begin() + i);
                positions.erase(positions.begin() + i);
                angles.erase(angles.begin() + i);
                radii.erase(radii.begin() + i);
                return true;
            } else {
                return false;
            }
        } else {
            bool removed = false;
            bool allLeaves = true;
            assert(!children.empty());
            for (index i = 0; i < children.size(); i++) {
                if (!children[i].isLeaf)
                    allLeaves = false;
                if (children[i].removeContent(input, angle, R)) {
                    assert(!removed);
                    removed = true;
                }
            }
            if (removed)
                subTreeSize--;
            // coarsen?
            if (removed && allLeaves && size() < coarsenLimit) {
                // coarsen!!
                // why not assert empty containers and then insert directly?
                vector<T> allContent;
                vector<Point2DWithIndex<double>> allPositions;
                vector<double> allAngles;
                vector<double> allRadii;
                for (index i = 0; i < children.size(); i++) {
                    allContent.insert(allContent.end(), children[i].content.begin(),
                                      children[i].content.end());
                    allPositions.insert(allPositions.end(), children[i].positions.begin(),
                                        children[i].positions.end());
                    allAngles.insert(allAngles.end(), children[i].angles.begin(),
                                     children[i].angles.end());
                    allRadii.insert(allRadii.end(), children[i].radii.begin(),
                                    children[i].radii.end());
                }
                assert(subTreeSize == allContent.size());
                assert(subTreeSize == allPositions.size());
                assert(subTreeSize == allAngles.size());
                assert(subTreeSize == allRadii.size());
                children.clear();
                content.swap(allContent);
                positions.swap(allPositions);
                angles.swap(allAngles);
                radii.swap(allRadii);
                isLeaf = true;
            }

            return removed;
        }
    }

    /**
     * Check whether the region managed by this node lies outside of an Euclidean circle.
     *
     * @param query Center of the Euclidean query circle, given in Cartesian coordinates
     * @param radius Radius of the Euclidean query circle
     *
     * @return True if the region managed by this node lies completely outside of the circle
     */
    bool outOfReach(Point2DWithIndex<double> query, double radius) const {
        double phi, r;
        HyperbolicSpace::cartesianToPolar(query, phi, r);
        if (responsible(phi, r))
            return false;

        // if using native coordinates, call distance calculation
        if (!poincare)
            return hyperbolicDistances(phi, r).first > radius;

        // get four edge points
        double topDistance, bottomDistance, leftDistance, rightDistance;

        if (phi < leftAngle || phi > rightAngle) {
            topDistance = std::min(c.distance(query), d.distance(query));
        } else {
            topDistance = std::abs(r - maxR);
        }
        if (topDistance <= radius)
            return false;
        if (phi < leftAngle || phi > rightAngle) {
            bottomDistance = std::min(a.distance(query), b.distance(query));
        } else {
            bottomDistance = std::abs(r - minR);
        }
        if (bottomDistance <= radius)
            return false;

        double minDistanceR = r * std::cos(std::abs(phi - leftAngle));
        if (minDistanceR > minR && minDistanceR < maxR) {
            leftDistance = query.distance(HyperbolicSpace::polarToCartesian(phi, minDistanceR));
        } else {
            leftDistance = std::min(a.distance(query), d.distance(query));
        }
        if (leftDistance <= radius)
            return false;

        minDistanceR = r * std::cos(std::abs(phi - rightAngle));
        if (minDistanceR > minR && minDistanceR < maxR) {
            rightDistance = query.distance(HyperbolicSpace::polarToCartesian(phi, minDistanceR));
        } else {
            rightDistance = std::min(b.distance(query), c.distance(query));
        }
        if (rightDistance <= radius)
            return false;
        return true;
    }

    /**
     * Check whether the region managed by this node lies outside of an Euclidean circle.
     * Functionality is the same as in the method above, but it takes polar coordinates instead of
     * Cartesian ones
     *
     * @param angle_c Angular coordinate of the Euclidean query circle's center
     * @param r_c Radial coordinate of the Euclidean query circle's center
     * @param radius Radius of the Euclidean query circle
     *
     * @return True if the region managed by this node lies completely outside of the circle
     */
    bool outOfReach(double angle_c, double r_c, double radius) const {
        if (responsible(angle_c, r_c))
            return false;
        Point2DWithIndex<double> query = HyperbolicSpace::polarToCartesian(angle_c, r_c);
        return outOfReach(query, radius);
    }

    /**
     * @param phi Angular coordinate of query point
     * @param r_h radial coordinate of query point in poincare disk
     */
    std::pair<double, double> hyperbolicDistances(double phi, double r) const {
        double minRHyper, maxRHyper, r_h;
        if (poincare) {
            minRHyper = HyperbolicSpace::EuclideanRadiusToHyperbolic(this->minR);
            maxRHyper = HyperbolicSpace::EuclideanRadiusToHyperbolic(this->maxR);
            r_h = HyperbolicSpace::EuclideanRadiusToHyperbolic(r);
        } else {
            minRHyper = this->minR;
            maxRHyper = this->maxR;
            r_h = r;
        }

        double coshr = std::cosh(r_h);
        double sinhr = std::sinh(r_h);
        double coshMinR = std::cosh(minRHyper);
        double coshMaxR = std::cosh(maxRHyper);
        double sinhMinR = std::sinh(minRHyper);
        double sinhMaxR = std::sinh(maxRHyper);
        double cosDiffLeft = std::cos(phi - leftAngle);
        double cosDiffRight = std::cos(phi - rightAngle);

        /**
         * If the query point is not within the quadnode, the distance minimum is on the border.
         * Need to check whether extremum is between corners:
         */

        double coshMinDistance, coshMaxDistance;

        // Left border
        double lowerLeftDistance = coshMinR * coshr - sinhMinR * sinhr * cosDiffLeft;
        double upperLeftDistance = coshMaxR * coshr - sinhMaxR * sinhr * cosDiffLeft;
        if (responsible(phi, r))
            coshMinDistance = 1; // strictly speaking, this is wrong
        else
            coshMinDistance = std::min(lowerLeftDistance, upperLeftDistance);

        coshMaxDistance = std::max(lowerLeftDistance, upperLeftDistance);
        // double a = std::cosh(r_h);
        double b = sinhr * cosDiffLeft;
        double extremum = std::log((coshr + b) / (coshr - b)) / 2;
        if (extremum < maxRHyper && extremum >= minRHyper) {
            double extremeDistance =
                std::cosh(extremum) * coshr - std::sinh(extremum) * sinhr * cosDiffLeft;
            coshMinDistance = std::min(coshMinDistance, extremeDistance);
            coshMaxDistance = std::max(coshMaxDistance, extremeDistance);
        }
        /**
         * cosh is a function from [0,\infty) to [1, \infty)
         * Variables thus need
         */
        assert(coshMaxDistance >= 1);
        assert(coshMinDistance >= 1);

        // Right border
        double lowerRightDistance = coshMinR * coshr - sinhMinR * sinhr * cosDiffRight;
        double upperRightDistance = coshMaxR * coshr - sinhMaxR * sinhr * cosDiffRight;
        coshMinDistance = std::min(coshMinDistance, lowerRightDistance);
        coshMinDistance = std::min(coshMinDistance, upperRightDistance);
        coshMaxDistance = std::max(coshMaxDistance, lowerRightDistance);
        coshMaxDistance = std::max(coshMaxDistance, upperRightDistance);

        b = sinhr * cosDiffRight;
        extremum = std::log((coshr + b) / (coshr - b)) / 2;
        if (extremum < maxRHyper && extremum >= minRHyper) {
            double extremeDistance =
                std::cosh(extremum) * coshr - std::sinh(extremum) * sinhr * cosDiffRight;
            coshMinDistance = std::min(coshMinDistance, extremeDistance);
            coshMaxDistance = std::max(coshMaxDistance, extremeDistance);
        }

        assert(coshMaxDistance >= 1);
        assert(coshMinDistance >= 1);

        // upper and lower borders
        if (phi >= leftAngle && phi < rightAngle) {
            double lower = std::cosh(std::abs(r_h - minRHyper));
            double upper = std::cosh(std::abs(r_h - maxRHyper));
            coshMinDistance = std::min(coshMinDistance, lower);
            coshMinDistance = std::min(coshMinDistance, upper);
            coshMaxDistance = std::max(coshMaxDistance, upper);
            coshMaxDistance = std::max(coshMaxDistance, lower);
        }

        assert(coshMaxDistance >= 1);
        assert(coshMinDistance >= 1);

        // again with mirrored phi
        double mirrorphi;
        if (phi >= PI)
            mirrorphi = phi - PI;
        else
            mirrorphi = phi + PI;
        if (mirrorphi >= leftAngle && mirrorphi < rightAngle) {
            double lower = coshMinR * coshr + sinhMinR * sinhr;
            double upper = coshMaxR * coshr + sinhMaxR * sinhr;
            coshMinDistance = std::min(coshMinDistance, lower);
            coshMinDistance = std::min(coshMinDistance, upper);
            coshMaxDistance = std::max(coshMaxDistance, upper);
            coshMaxDistance = std::max(coshMaxDistance, lower);
        }

        assert(coshMaxDistance >= 1);
        assert(coshMinDistance >= 1);

        double minDistance, maxDistance;
        minDistance = std::acosh(coshMinDistance);
        maxDistance = std::acosh(coshMaxDistance);
        assert(maxDistance >= 0);
        assert(minDistance >= 0);
        return std::pair<double, double>(minDistance, maxDistance);
    }

    /**
     * Does the point at (angle, r) fall inside the region managed by this QuadNode?
     *
     * @param angle Angular coordinate of input point
     * @param r Radial coordinate of input points
     *
     * @return True if input point lies within the region of this QuadNode
     */
    bool responsible(double angle, double r) const {
        return (angle >= leftAngle && angle < rightAngle && r >= minR && r < maxR);
    }

    /**
     * Get all Elements in this QuadNode or a descendant of it
     *
     * @return vector of content type T
     */
    std::vector<T> getElements() const {
        if (isLeaf) {
            return content;
        } else {
            assert(content.size() == 0);
            assert(angles.empty());
            assert(radii.empty());
            vector<T> result;
            for (index i = 0; i < children.size(); i++) {
                std::vector<T> subresult = children[i].getElements();
                result.insert(result.end(), subresult.begin(), subresult.end());
            }
            return result;
        }
    }

    void getCoordinates(vector<double> &anglesContainer, vector<double> &radiiContainer) const {
        assert(angles.size() == radii.size());
        if (isLeaf) {
            anglesContainer.insert(anglesContainer.end(), angles.begin(), angles.end());
            radiiContainer.insert(radiiContainer.end(), radii.begin(), radii.end());
        } else {
            assert(content.size() == 0);
            assert(angles.empty());
            assert(radii.empty());
            for (index i = 0; i < children.size(); i++) {
                children[i].getCoordinates(anglesContainer, radiiContainer);
            }
        }
    }

    /**
     * Don't use this!
     * Code is still in here for a unit test.
     *
     * Get copy of the leaf cell responsible for a point at (angle, r).
     * Expensive because it copies the whole subtree, causes assertion failure if called with the
     * wrong arguments
     *
     * @param angle Angular coordinate of point
     * @param r Radial coordinate of point
     *
     * @return Copy of leaf cell containing point, or dummy cell not responsible for point
     *
     */
    QuadNode<T> &getAppropriateLeaf(double angle, double r) {
        assert(this->responsible(angle, r));
        if (isLeaf)
            return *this; // will this return the reference to the subtree itself or to a copy?
        else {
            for (index i = 0; i < children.size(); i++) {
                if (children[i].responsible(angle, r))
                    return children[i].getAppropriateLeaf(angle, r);
            }
            throw std::runtime_error("No responsible child found.");
        }
    }

    /**
     * Main query method, get points lying in a Euclidean circle around the center point.
     * Optional limits can be given to get a different result or to reduce unnecessary comparisons
     *
     * Elements are pushed onto a vector which is a required argument. This is done to reduce
     * copying
     *
     * Safe to call in parallel if diagnostics are disabled
     *
     * @param center Center of the query circle
     * @param radius Radius of the query circle
     * @param result Reference to the vector where the results will be stored
     * @param minAngle Optional value for the minimum angular coordinate of the query region
     * @param maxAngle Optional value for the maximum angular coordinate of the query region
     * @param lowR Optional value for the minimum radial coordinate of the query region
     * @param highR Optional value for the maximum radial coordinate of the query region
     */
    void getElementsInEuclideanCircle(Point2DWithIndex<double> center, double radius,
                                      vector<T> &result, double minAngle = 0,
                                      double maxAngle = 2 * PI, double lowR = 0,
                                      double highR = 1) const {
        if (!poincare)
            throw std::runtime_error(
                "Euclidean query circles not yet implemented for native hyperbolic coordinates.");
        if (minAngle >= rightAngle || maxAngle <= leftAngle || lowR >= maxR || highR < lowerBoundR)
            return;
        if (outOfReach(center, radius)) {
            return;
        }

        if (isLeaf) {
            const double rsq = radius * radius;
            const double queryX = center[0];
            const double queryY = center[1];
            const count cSize = content.size();

            for (index i = 0; i < cSize; i++) {
                const double deltaX = positions[i].getX() - queryX;
                const double deltaY = positions[i].getY() - queryY;
                if (deltaX * deltaX + deltaY * deltaY < rsq) {
                    result.push_back(content[i]);
                }
            }
        } else {
            for (index i = 0; i < children.size(); i++) {
                children[i].getElementsInEuclideanCircle(center, radius, result, minAngle, maxAngle,
                                                         lowR, highR);
            }
        }
    }

    count getElementsProbabilistically(Point2DWithIndex<double> euQuery,
                                       std::function<double(double)> prob, bool suppressLeft,
                                       vector<T> &result) const {
        double phi_q, r_q;
        HyperbolicSpace::cartesianToPolar(euQuery, phi_q, r_q);
        if (suppressLeft && phi_q > rightAngle)
            return 0;
        TRACE("Getting hyperbolic distances");
        auto distancePair = hyperbolicDistances(phi_q, r_q);
        double probUB = prob(distancePair.first);
        double probLB = prob(distancePair.second);
#ifndef NDEBUG
        assert(probLB <= probUB);
#else
        ((void)(probLB));
#endif // NDEBUG
        if (probUB > 0.5)
            // if we are going to take every second element anyway, no use in calculating expensive
            // jumps
            probUB = 1;
        if (probUB == 0)
            return 0;
        // TODO: return whole if probLB == 1
        double probdenom = std::log(1 - probUB);
        if (probdenom == 0) {
            DEBUG(probUB, " not zero, but too small too process. Ignoring.");
            return 0;
        }
        TRACE("probUB: ", probUB, ", probdenom: ", probdenom);

        count expectedNeighbours = probUB * size();
        count candidatesTested = 0;

        if (isLeaf) {
            const count lsize = content.size();
            TRACE("Leaf of size ", lsize);
            for (index i = 0; i < lsize; i++) {
                // jump!
                if (probUB < 1) {
                    double random = Aux::Random::real();
                    double delta = std::log(random) / probdenom;
                    assert(delta == delta);
                    assert(delta >= 0);
                    i += delta;
                    if (i >= lsize)
                        break;
                    TRACE("Jumped with delta ", delta, " arrived at ", i);
                }

                // see where we've arrived
                candidatesTested++;
                double distance;
                if (poincare) {
                    distance = HyperbolicSpace::poincareMetric(positions[i], euQuery);
                } else {
                    distance = HyperbolicSpace::nativeDistance(angles[i], radii[i], phi_q, r_q);
                }
                assert(distance >= distancePair.first);

                double q = prob(distance);
                // since the candidate was selected by the jumping process, we have to adjust the
                // probabilities
                q = q / probUB;

                assert(q <= 1);
                assert(q >= 0);

                // accept?
                double acc = Aux::Random::real();
                if (acc < q) {
                    TRACE("Accepted node ", i, " with probability ", q, ".");
                    result.push_back(content[i]);
                }
            }
        } else {
            if (expectedNeighbours
                < 1) { // select candidates directly instead of calling recursively
                TRACE("probUB = ", probUB, ", switching to direct candidate selection.");
                assert(probUB < 1);
                const count stsize = size();
                for (index i = 0; i < stsize; i++) {
                    double delta = std::log(Aux::Random::real()) / probdenom;
                    assert(delta >= 0);
                    i += delta;
                    TRACE("Jumped with delta ", delta, " arrived at ", i,
                          ". Calling maybeGetKthElement.");
                    if (i < size())
                        // this could be optimized. As of now, the offset is subtracted  separately
                        // for each point
                        maybeGetKthElement(probUB, euQuery, prob, i, result);

                    else
                        break;
                    candidatesTested++;
                }
            } else { // carry on as normal
                for (index i = 0; i < children.size(); i++) {
                    TRACE("Recursively calling child ", i);
                    candidatesTested += children[i].getElementsProbabilistically(
                        euQuery, prob, suppressLeft, result);
                }
            }
        }
        return candidatesTested;
    }

    void maybeGetKthElement(double upperBound, Point2DWithIndex<double> euQuery,
                            std::function<double(double)> prob, index k,
                            vector<T> &circleDenizens) const {
        TRACE("Maybe get element ", k, " with upper Bound ", upperBound);
        assert(k < size());
        if (isLeaf) {
            double distance;
            if (poincare) {
                distance = HyperbolicSpace::poincareMetric(positions[k], euQuery);
            } else {
                double phi_q, r_q;
                HyperbolicSpace::cartesianToPolar(euQuery, phi_q, r_q);
                distance = HyperbolicSpace::nativeDistance(angles[k], radii[k], phi_q, r_q);
            }
            double acceptance = prob(distance) / upperBound;
            TRACE("Is leaf, accept with ", acceptance);
            if (Aux::Random::real() < acceptance)
                circleDenizens.push_back(content[k]);
        } else {
            TRACE("Call recursively.");
            index offset = 0;
            for (index i = 0; i < children.size(); i++) {
                count childsize = children[i].size();
                if (k - offset < childsize) {
                    children[i].maybeGetKthElement(upperBound, euQuery, prob, k - offset,
                                                   circleDenizens);
                    break;
                }
                offset += childsize;
            }
        }
    }

    /**
     * Shrink all vectors in this subtree to fit the content.
     * Call after quadtree construction is complete, causes better memory usage and cache efficiency
     */
    void trim() {
        content.shrink_to_fit();
        positions.shrink_to_fit();
        angles.shrink_to_fit();
        radii.shrink_to_fit();
        if (!isLeaf) {
            for (index i = 0; i < children.size(); i++) {
                children[i].trim();
            }
        }
    }

    /**
     * Number of points lying in the region managed by this QuadNode
     */
    count size() const { return isLeaf ? content.size() : subTreeSize; }

    void recount() {
        subTreeSize = 0;
        for (index i = 0; i < children.size(); i++) {
            children[i].recount();
            subTreeSize += children[i].size();
        }
    }

    /**
     * Height of subtree hanging from this QuadNode
     */
    count height() const {
        count result = 1; // if leaf node, the children loop will not execute
        for (auto child : children)
            result = std::max(result, child.height() + 1);
        return result;
    }

    /**
     * Leaf cells in the subtree hanging from this QuadNode
     */
    count countLeaves() const {
        if (isLeaf)
            return 1;
        return std::accumulate(
            children.begin(), children.end(), count{0},
            [](count result, const auto &child) -> count { return result + child.countLeaves(); });
    }

    double getLeftAngle() const { return leftAngle; }

    double getRightAngle() const { return rightAngle; }

    double getMinR() const { return minR; }

    double getMaxR() const { return maxR; }

    index getID() const { return ID; }

    index indexSubtree(index nextID) {
        index result = nextID;
        assert(children.size() == 4 || children.size() == 0);
        for (index i = 0; i < children.size(); i++) {
            result = children[i].indexSubtree(result);
        }
        this->ID = result;
        return result + 1;
    }

    index getCellID(double phi, double r) const {
        if (!responsible(phi, r))
            return none;
        if (isLeaf)
            return getID();
        else {
            for (index i = 0; i < children.size(); i++) {
                index childresult = children[i].getCellID(phi, r);
                if (childresult != none)
                    return childresult;
            }
            throw std::runtime_error(
                "No responsible child node found even though this node is responsible.");
        }
    }

    index getMaxIDInSubtree() const {
        if (isLeaf)
            return getID();
        else {
            index result = -1;
            for (int i = 0; i < 4; i++) {
                result = std::max(children[i].getMaxIDInSubtree(), result);
            }
            return std::max(result, getID());
        }
    }

    count reindex(count offset) {
        if (isLeaf) {
#ifndef NETWORKIT_OMP2
#pragma omp task
#endif // NETWORKIT_OMP2
            {
                index p = offset;
                std::generate(content.begin(), content.end(), [&p]() { return p++; });
            }
            offset += size();
        } else {
            for (int i = 0; i < 4; i++) {
                offset = children[i].reindex(offset);
            }
        }
        return offset;
    }
};
} // namespace NetworKit

#endif // NETWORKIT_GENERATORS_QUADTREE_QUAD_NODE_HPP_
