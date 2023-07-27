#include <stack>
#include <unordered_set>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <math.h>

typedef int intE;
typedef unsigned int uintE;

namespace distributed_kcore {
inline double group_degree(size_t group, double phi) {
    return pow(1 + phi, group);
}

inline double upper_constant(double delta, bool optimized) {
    if (optimized) {
        return 1.1;
    } else {
        return (2 + static_cast<double>(3) /  delta);
    }
}

struct LDS {
    size_t n; // number of vertices
    double delta = 9.0;
    double phi = 3.0;
    bool optimized_insertion = false;

    struct LDSVertex {                
            uintE level; // level of this vertex
            LDSVertex() : level(0) {}
    };

    size_t levels_per_group; // number of inner-levels per group. O(\log n) many
    std::vector<LDSVertex> L;

    LDS(size_t _n, double _eps, double _delta, bool _optimized) : n(_n), phi(_eps),
        delta(_delta), optimized_insertion(_optimized) {
            levels_per_group = ceil(log(n) / log(1 + phi));
            L = std::vector<LDSVertex>(n);
    }

    uintE get_level(uintE ngh) {
        return L[ngh].level;
    }
    // Moving u from level to level + 1.
    /**
     * bookkeeping struct, which vertices need to move for current u
     * L -> array to access LDS array easily 
    */
    template <class Levels>
    void level_increase_v2(uintE u, Levels& L) {
        L[u].level++;
    }

    inline uintE group_for_level(uintE level) const {
        return level / levels_per_group;
    }
};
} // end of namespace distributed_kcore