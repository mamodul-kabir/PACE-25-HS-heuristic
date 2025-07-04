
#include <fstream>

#include <networkit/io/CoverWriter.hpp>

namespace NetworKit {

void CoverWriter::write(Cover &zeta, std::string_view path) const {
    std::ofstream file{path.data()};

    std::vector<std::vector<index>> sets(zeta.upperBound());
    zeta.forEntries([&](index v, const std::set<index> &c) {
        for (auto &s : c) {
            sets[s].push_back(v);
        }
    });

    for (auto &nodes : sets) {
        for (auto &v : nodes) {
            file << v << " ";
        }
        file << '\n';
    }
}

} // namespace NetworKit
