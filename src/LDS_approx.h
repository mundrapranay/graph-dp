#include <stack>
#include <unordered_set>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <parlay/delayed_sequence.h>
#include <parlay/sequence.h>

typedef int intE;
typedef unsigned int uintE;

struct Level {
  std::unordered_set<uintE> set;
  std::vector<uintE> vector;
  static constexpr int high_boundary = 1000;
  static constexpr int low_boundary = 500;
  bool use_vector;

  Level() : use_vector(true) {}

  void set_to_vector() {
    for (const auto& v : set) {
      vector.push_back(v);
    }
    set.clear();
    use_vector = true;
  }

  void vector_to_set() {
    for (const auto& v : vector) {
      set.insert(v);
    }
    vector.clear();
    use_vector = false;
  }

  void erase(uintE v) {
    if (use_vector) vector.erase(std::find(vector.begin(), vector.end(), v));
    else {
      set.erase(set.find(v));
      if (set.size() < low_boundary) set_to_vector();
    }
  }

  template <class Iterator>
  void erase(Iterator it) {
    if (use_vector) vector.erase(it);
    else {
      set.erase(it);
      if (set.size() < low_boundary) set_to_vector();
    }
  }

  void insert(uintE v) {
    if (use_vector) {
      vector.push_back(v);
      if (vector.size() > high_boundary) vector_to_set();
    } else set.insert(v);
  }

  template <class F>
  void iterate(F f) {
    if (use_vector) {
      for (const auto& v : vector) {
        f(v);
      }
    } else {
      for (const auto& v : set) {
        f(v);
      }
    }
  }

  template <class F>
  void special_iterate(F f) {
    if (use_vector) {
      auto end = set.end();
      for (auto it = vector.begin(); it != vector.end();) {
        f(it, end);
      }
    } else {
      auto end = vector.end();
      for (auto it = set.begin(); it != set.end();) {
        f(end, it);
      }
    }
  }

  size_t size() const {
    if (use_vector) return vector.size();
    return set.size();
  }
};

inline double group_degree(size_t group, double epsilon) {
  return pow(1 + epsilon, group);
}

inline double upper_constant(double delta, bool optimized) {
  if (optimized)
    return 1.1;
  else
    return (2 + static_cast<double>(3) / delta);
}

struct LDS {
  size_t n;  // number of vertices
  double delta = 9.0;
  double epsilon = 3.0;
  bool optimized_insertion = false;

  size_t total_work;

  //using level = std::unordered_set<uintE>;
  using down_neighbors = std::vector<Level>;
  using up_neighbors = Level;
  using edge_type = std::pair<uintE, uintE>;

  struct LDSVertex {
    uintE level;  // level of this vertex
    down_neighbors
        down;         // neighbors in levels < level, bucketed by their level.
    up_neighbors up;  // neighbors

    LDSVertex() : level(0) {}

    void insert_neighbor(uintE v, uintE l_v) {
      if (l_v < level) {
        assert(down.size() > l_v);
        down[l_v].insert(v);
      } else {
        up.insert(v);
      }
    }

    void remove_neighbor(uintE v, uintE l_v) {
      if (l_v < level) {
        down[l_v].erase(v);
      } else {
        up.erase(v);
      }
    }


    inline bool upper_invariant(const size_t levels_per_group, double epsilon, double delta,
            bool optimized) const {
      uintE group = level / levels_per_group;
      return up.size() <=
             static_cast<size_t>(upper_constant(delta, optimized) * group_degree(group, epsilon));
    }

    inline bool lower_invariant(const size_t levels_per_group, double epsilon) const {
      if (level == 0) return true;
      uintE lower_group = (level - 1) / levels_per_group;
      auto up_size = up.size();
      auto prev_level_size = down[level - 1].size();
      return (up_size + prev_level_size) >=
             static_cast<size_t>(
                 group_degree(lower_group, epsilon));
    }
  };

  // number of inner-levels per group,  O(\log n) many.
  size_t levels_per_group;
  // parlay::sequence<LDSVertex> L;
  std::vector<LDSVertex> L;
  std::stack<uintE> Dirty;

  LDS(size_t _n, double _eps, double _delta, bool _optimized) : n(_n), epsilon(_eps),
    delta(_delta), optimized_insertion(_optimized) {
    levels_per_group = ceil(log(n) / log(1 + epsilon));
    // levels_per_group = parlay::log2_up(n);
    // L = parlay::sequence<LDSVertex>(n);
    L =  std::vector<LDSVertex>(n);
  }

  uintE get_level(uintE ngh) { return L[ngh].level; }

  // Moving u from level to level - 1.
  template <class Levels>
  void level_decrease(uintE u, Levels& L) {
    total_work++;
    uintE level = L[u].level;
    auto& up = L[u].up;
    assert(level > 0);
    auto& prev_level = L[u].down[level - 1];

    prev_level.iterate([&](const uintE& ngh) {
      up.insert(ngh);
    });
    L[u].down.pop_back();  // delete the last level in u's structure.

    up.iterate([&](const uintE& ngh) {
    //for (const auto& ngh : up) {
      if (get_level(ngh) == level) {
        L[ngh].up.erase(u);
        L[ngh].down[level - 1].insert(u);

      } else if (get_level(ngh) > level) {
        L[ngh].down[level].erase(u);
        L[ngh].down[level - 1].insert(u);

        if (get_level(ngh) == level + 1) {
          Dirty.push(ngh);
        }
      } else {
        // u is still "up" for this ngh, no need to update.
        assert(get_level(ngh) == (level - 1));
      }
    });
    L[u].level--;  // decrease level
  }

  // Moving u from level to level + 1.
  template <class Levels>
  void level_increase(uintE u, Levels& L) {
    total_work++;
    uintE level = L[u].level;
    std::vector<uintE> same_level;
    auto& up = L[u].up;

    up.special_iterate([&](std::vector<uintE>::iterator& vec_it,
      std::unordered_set<uintE>::iterator& set_it) {
      bool use_vec = (vec_it != up.vector.end());
      uintE ngh = use_vec ? *vec_it : *set_it; //*it;
      if (L[ngh].level == level) {
        same_level.emplace_back(ngh);
        if (use_vec) vec_it = up.vector.erase(vec_it);
        else set_it = up.set.erase(set_it);
        // u is still "up" for this ngh, no need to update.
      } else {
        if (use_vec) vec_it++;
        else set_it++;
        // Must update ngh's accounting of u.
        if (L[ngh].level > level + 1) {
          L[ngh].down[level].erase(u);
          L[ngh].down[level + 1].insert(u);
        } else {
          assert(L[ngh].level == level + 1);
          L[ngh].down[level].erase(u);
          L[ngh].up.insert(u);

          Dirty.push(ngh);
        }
      }
    });
    // We've now split L[u].up into stuff in the same level (before the
    // update) and stuff in levels >= level + 1. Insert same_level elms
    // into down.
    auto& down = L[u].down;
    down.emplace_back(Level());
    assert(down.size() == level + 1);  // [0, level)
    for (const auto& ngh : same_level) {
      down[level].insert(ngh);
    }
    L[u].level++;  // Increase level.
  }

  void fixup() {
    while (!Dirty.empty()) {
      uintE u = Dirty.top();
      Dirty.pop();
      if (!L[u].upper_invariant(levels_per_group, epsilon, delta, optimized_insertion)) {
        // Move u to level i+1.
        level_increase(u, L);
        Dirty.push(u);  // u might need to move up more levels.
      } else if (!L[u].lower_invariant(levels_per_group, epsilon)) {
        level_decrease(u, L);
        Dirty.push(u);  // u might need to move down more levels.
      }
    }
  }

  bool insert_edge(edge_type e) {
    auto[u, v] = e;
    auto l_u = L[u].level;
    auto l_v = L[v].level;
    L[u].insert_neighbor(v, l_v);
    L[v].insert_neighbor(u, l_u);

    Dirty.push(u);
    Dirty.push(v);
    fixup();
    return true;
  }

  bool delete_edge(edge_type e) {
    auto[u, v] = e;
    auto l_u = L[u].level;
    auto l_v = L[v].level;
    L[u].remove_neighbor(v, l_v);
    L[v].remove_neighbor(u, l_u);

    Dirty.push(u);
    Dirty.push(v);
    fixup();
    return true;
  }

  void check_invariants() {
    bool invs_ok = true;
    for (size_t i = 0; i < n; i++) {
      bool upper_ok = L[i].upper_invariant(levels_per_group, epsilon, delta, optimized_insertion);
      bool lower_ok = L[i].lower_invariant(levels_per_group, epsilon);
      assert(upper_ok);
      assert(lower_ok);
      invs_ok &= upper_ok;
      invs_ok &= lower_ok;
    }
  }

  // uintE max_coreness() {
  //   auto levels = parlay::delayed_seq<uintE>(n, [&] (size_t i) { return L[i].level; });
  //   uintE max_level = pbbslib::reduce_max(levels);
  //   uintE max_group = group_for_level(max_level);
  //   return group_degree(max_group, epsilon);
  // }

  uintE core(uintE v) {
    auto l = L[v].level;
    uintE group = group_for_level(l);
    // If l is not the highest level in the group, we drop to the previous group
    if (l % levels_per_group != levels_per_group - 1 && group != 0) group--;
    return ceil(group_degree(group, epsilon));
  }

  inline uintE group_for_level(uintE level) const {
    return level / levels_per_group;
  }

  size_t get_size() {
    size_t size = 0;
    size += sizeof(delta) + sizeof(epsilon) + sizeof(optimized_insertion)
        + sizeof(levels_per_group) + sizeof(n);

    for (size_t i = 0; i < n; i++) {
        auto vertex = L[i];
        size += sizeof(vertex.level);
        for (size_t j = 0; j < vertex.down.size(); j++) {
            auto level = vertex.down[j];
            for (size_t k = 0; k < level.vector.size(); k++)
                size += sizeof(level.vector[k]);
            for (const auto& elem: level.set)
                size += sizeof(elem);
        }

        auto level = vertex.up;
        for (size_t k = 0; k < level.vector.size(); k++)
            size += sizeof(level.vector[k]);
        for (const auto& elem: level.set)
            size += sizeof(elem);
    }
    return size;
  }
};


