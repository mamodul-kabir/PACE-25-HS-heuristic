/*
 * SNAPEdgeListPartitionReader.cpp
 *
 *  Created on: Jun 20, 2013
 *      Author: forigem
 */

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <networkit/auxiliary/Log.hpp>
#include <networkit/auxiliary/StringTools.hpp>
#include <networkit/io/SNAPEdgeListPartitionReader.hpp>

#include <tlx/unused.hpp>

namespace NetworKit {

Cover SNAPEdgeListPartitionReader::read(std::string_view path,
                                        std::unordered_map<node, node> &mapNodeIds, Graph &G) {
    std::ifstream file;
    std::string line; // the current line

    // read file once to get to the last line and figure out the number of nodes
    // unfortunately there is an empty line at the ending of the file, so we need to get the line
    // before that

    file.open(path.data());
    if (!file.good()) {
        throw std::runtime_error("unable to read from file");
    }

    node current;

    std::string commentPrefix = "#";

#ifndef NDEBUG
    std::unordered_set<node> uniqueIDs;
#endif
    count totalCounter = 0;
    tlx::unused(totalCounter);
    Cover communities(G.upperNodeIdBound());

    // first find out the maximum node id
    count i = 0;
    while (file.good()) {
        ++i;
        std::getline(file, line);
        if (line.compare(0, commentPrefix.length(), commentPrefix) == 0) {
        } else if (line.length() == 0) {
        } else {
            std::stringstream linestream(line);
            while (linestream >> current) {
#ifndef NDEBUG
                uniqueIDs.insert(current);
#endif
                ++totalCounter;
                if (mapNodeIds.find(current) != mapNodeIds.end()) {
                    communities.addToSubset(i, mapNodeIds[current]);
                } else {
                    WARN("unknown node ID found (", current, ") and ignored");
                }
            }
        }
    }
#ifndef NDEBUG
    DEBUG("read ", uniqueIDs.size(),
          " unique node IDs with the total amount of occurrences: ", totalCounter);
#endif
    tlx::unused(totalCounter);
    count emptyElements = 0;
    count output = 0;
    std::stringstream outputString;
    std::vector<node> outputIDs;
    outputString << "first 10 unassigned IDs: ";
    for (index i = 0, end = communities.numberOfElements(); i < end; ++i) {
        if (communities[i].empty()) {
            ++emptyElements;
            if (output < 10) {
                outputIDs.push_back(i);
                output++;
            }
        }
    }
    DEBUG(emptyElements, " nodes have not been assigned to any community");
    tlx::unused(emptyElements);
    auto endIt = mapNodeIds.end();
    for (index i = 0, end = outputIDs.size(); i < end; ++i) {
        bool found = false;
        auto it = mapNodeIds.begin();
        // DEBUG("find key to nodeID: ",outputIDs[i]);
        while (!found && it != endIt) {
            if (it->second == outputIDs[i]) {
                found = true;
                // DEBUG("key is: ",it->first);
                outputString << "(" << it->first << "," << it->second << "),\t";
            }
            ++it;
        }
    }
    DEBUG(outputString.str());

    file.close();
    communities.setUpperBound(i);
    return communities;
}

} /* namespace NetworKit */
