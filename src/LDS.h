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

// struct Level {
//     std::unordered_set<uintE> set;
//     std::vector<uintE> vector;
//     static constexpr int high_boundary = 1000;
//     static constexpr int low_boundary = 500;
//     bool use_vector;
//     Level() : use_vector(true) {}

//     void set_to_vector() {
//         for (const auto& v : set) {
//             vector.push_back(v);
//         }
//         set.clear();
//         use_vector = true;
//     }

//     void vector_to_set() {
//         for (const auto& v : vector) {
//             set.insert(v);
//         }
//         vector.clear();
//         use_vector = false;
//     }

//     void erase(uintE v) {
//         if (use_vector) {
//             vector.erase(std::find(vector.begin(), vector.end(), v));
//         } else {
//             set.erase(set.find(v));
//             if (set.size() < low_boundary) {
//                 set_to_vector();
//             }
//         }
//     }

//     template <class Iterator>
//     void erase(Iterator it) {
//         if (use_vector) {
//             vector.erase(it);
//         } else {
//             set.erase(it);
//             if (set.size() < low_boundary) {
//                 set_to_vector();
//             }
//         }
//     }

//     void insert(uintE v) {
//         if (use_vector) {
//             vector.push_back(v);
//             if (vector.size() > high_boundary) {
//                 vector_to_set();
//             }
//         } else {
//             set.insert(v);
//         }
//     }

//     template <class F>
//     void iterate(F f) {
//         if (use_vector) {
//         for (const auto& v : vector) {
//             f(v);
//         }
//         } else {
//         for (const auto& v : set) {
//             f(v);
//         }
//         }
//     }

//     template <class F>
//     void special_iterate(F f) {
//         if (use_vector) {
//         auto end = set.end();
//         for (auto it = vector.begin(); it != vector.end();) {
//             f(it, end);
//         }
//         } else {
//         auto end = vector.end();
//         for (auto it = set.begin(); it != set.end();) {
//             f(end, it);
//         }
//         }
//     }

//     size_t size() const {
//         if (use_vector) {
//             return vector.size();
//         }
//         return set.size();
//     }
// };



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