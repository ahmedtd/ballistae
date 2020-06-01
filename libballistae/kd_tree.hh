#ifndef LIBBALLISTAE_KD_TREE_HH
#define LIBBALLISTAE_KD_TREE_HH

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cstddef>
#include <memory>
#include <vector>

#include "libballistae/aabox.hh"
#include "libballistae/span.hh"

namespace ballistae {

/// The LINK field is an index into the kd tree's NODES field, and points to a
/// the source of a two-node block holding the children of this node.
template <typename Stored>
struct aanode {
  aabox bounds;

  typename std::vector<Stored>::iterator elements_src;
  typename std::vector<Stored>::iterator elements_lim;

  std::unique_ptr<aanode<Stored>> lo_child;
  std::unique_ptr<aanode<Stored>> hi_child;
};

/// A balanced, bounded-volume
template <typename Stored>
struct kd_tree final {
  std::vector<Stored> infinite_elements;
  std::vector<Stored> finite_elements;
  std::unique_ptr<aanode<Stored>> root;

  kd_tree() = default;

  kd_tree(const kd_tree<Stored> &other) = delete;
  kd_tree(kd_tree<Stored> &&other) = default;

  template <typename StoredToAABox>
  kd_tree(std::vector<Stored> &&storage_in, StoredToAABox get_aabox);

  template <typename Selector, typename Computor>
  void query(Selector selector, Computor computor) const;

  kd_tree<Stored> &operator=(const kd_tree<Stored> &other) = delete;
  kd_tree<Stored> &operator=(kd_tree<Stored> &&other) = default;
};

template <typename Stored>
template <typename StoredToAABox>
kd_tree<Stored>::kd_tree(std::vector<Stored> &&storage,
                         StoredToAABox get_aabox) {
  using std::begin;
  using std::end;

  using std::make_move_iterator;

  // Split the provided elements into finite and infinite elements.
  auto finite_lim = std::partition(begin(storage), end(storage), [&](auto &x) {
    return isfinite(get_aabox(x));
  });
  finite_elements = std::vector<Stored>(make_move_iterator(begin(storage)),
                                        make_move_iterator(finite_lim));
  infinite_elements = std::vector<Stored>(make_move_iterator(finite_lim),
                                          make_move_iterator(end(storage)));

  aabox max_box = std::accumulate(
      std::begin(finite_elements), std::end(finite_elements),
      aabox::accum_zero(),
      [&](auto a, auto b) { return min_containing(a, get_aabox(b)); });

  // Root node encompasses all finite boxes.
  root = std::make_unique<aanode<Stored>>();
  *root = {max_box, begin(finite_elements), end(finite_elements), nullptr,
           nullptr};
}

template <typename Stored, typename StoredToAABox>
void kd_tree_refine_sah(aanode<Stored> *cur, StoredToAABox get_aabox,
                        double split_cost, double threshold) {
  using std::begin;
  using std::end;

  using std::make_move_iterator;

  using std::distance;

  std::mt19937 rng;

  std::vector<aanode<Stored> *> work_stack;
  work_stack.push_back(cur);

  while (!work_stack.empty()) {
    cur = work_stack.back();
    work_stack.pop_back();

    if (distance(cur->elements_src, cur->elements_lim) < 2) return;

    bool should_cut;
    size_t cut_axis;
    double cut;
    std::tie(should_cut, cut_axis, cut) =
        split_sah(*(cur), get_aabox, split_cost, threshold, rng);

    // Terminate recursion if indicated.
    if (!should_cut) continue;

    // Place elements that strictly precede the cut into the low child of the
    // current node.
    auto precede_src = cur->elements_src;
    auto precede_lim =
        std::partition(cur->elements_src, cur->elements_lim, [&](auto &a) {
          return strictly_precedes(get_aabox(a).spans[cut_axis], cut);
        });

    if (distance(precede_src, precede_lim) != 0) {
      aabox lo_bounds = std::accumulate(
          precede_src, precede_lim, aabox::accum_zero(),
          [&](auto a, auto b) { return min_containing(a, get_aabox(b)); });

      cur->lo_child = std::make_unique<aanode<Stored>>();
      *(cur->lo_child) = {lo_bounds, precede_src, precede_lim, nullptr,
                          nullptr};

      work_stack.push_back(cur->lo_child.get());
    }

    // All remaining elements overlap or strictly succeed the cut, and are
    // placed in the high child.

    auto succeed_src = precede_lim;
    auto succeed_lim = cur->elements_lim;

    if (distance(succeed_src, succeed_lim) != 0) {
      aabox hi_bounds = std::accumulate(
          succeed_src, succeed_lim, aabox::accum_zero(),
          [&](auto a, auto b) { return min_containing(a, get_aabox(b)); });

      cur->hi_child = std::make_unique<aanode<Stored>>();
      *(cur->hi_child) = {hi_bounds, succeed_src, succeed_lim, nullptr,
                          nullptr};

      work_stack.push_back(cur->hi_child.get());
    }
  }
}

template <typename Stored, typename StoredToAABox>
std::tuple<bool, size_t, double> split_sah(aanode<Stored> &cur,
                                           StoredToAABox get_aabox,
                                           double split_cost,
                                           double termination_threshold,
                                           std::mt19937 &rng) {
  using std::begin;
  using std::end;

  using std::distance;

  double parent_objective =
      distance(cur.elements_src, cur.elements_lim) * surface_area(cur.bounds);

  size_t piv_axis = 0;
  double piv_cut;
  double piv_objective = std::numeric_limits<double>::infinity();

  std::uniform_int_distribution<std::size_t> element_picker(
      0, std::distance(cur.elements_src, cur.elements_lim));
  for (size_t axis = 0; axis < 3; ++axis) {
    // Check five random elements from this axis
    for (std::size_t cur_check = 0; cur_check < 5; cur_check++) {
      auto it = cur.elements_src + element_picker(rng);

      auto split =
          std::partition(cur.elements_src, cur.elements_lim, [&](auto &e) {
            return strictly_precedes(get_aabox(e).spans[axis],
                                     get_aabox(*it).spans[axis].hi);
          });

      auto precede_src = cur.elements_src;
      auto precede_lim = split;

      auto succeed_src = split;
      auto succeed_lim = cur.elements_lim;

      aabox lo_box = std::accumulate(
          precede_src, precede_lim, aabox::accum_zero(),
          [&](auto a, auto b) { return min_containing(a, get_aabox(b)); });

      aabox hi_box = std::accumulate(
          succeed_src, succeed_lim, aabox::accum_zero(),
          [&](auto a, auto b) { return min_containing(a, get_aabox(b)); });

      double objective = double(0);
      if (distance(precede_src, precede_lim) != 0)
        objective += surface_area(lo_box) * distance(precede_src, precede_lim);
      if (distance(succeed_src, succeed_lim) != 0)
        objective += surface_area(hi_box) * distance(succeed_src, succeed_lim);

      if (objective < piv_objective) {
        piv_axis = axis;
        piv_cut = get_aabox(*it).spans[axis].hi;
        piv_objective = objective;
      }
    }
  }

  // Now we have computed the optimal split.  We need to check if it is a good
  // improvement over simply not splitting.
  if (piv_objective + split_cost > termination_threshold * parent_objective) {
    return std::make_tuple(false, 0, double(0));
  } else {
    return std::make_tuple(true, piv_axis, piv_cut);
  }
}

template <typename Stored>
template <typename Selector, typename Computor>
void kd_tree<Stored>::query(Selector selector, Computor computor) const {
  using std::begin;
  using std::end;

  // Always check all infinite elements.
  std::for_each(begin(infinite_elements), end(infinite_elements), computor);

  // We recurse to a fixed depth.
  std::array<aanode<Stored> *, 2048> work_stack;

  auto top = begin(work_stack);
  auto base = begin(work_stack);

  *top = root.get();
  ++top;

  while (top != base) {
    --top;
    auto p_cur = *top;

    if (selector(p_cur->bounds)) {
      if (p_cur->lo_child == nullptr && p_cur->hi_child == nullptr) {
        std::for_each(p_cur->elements_src, p_cur->elements_lim, computor);
      } else if (static_cast<size_t>(top - base) < work_stack.size() - 2) {
        if (p_cur->lo_child != nullptr) {
          *top = p_cur->lo_child.get();
          ++top;
        }
        if (p_cur->hi_child != nullptr) {
          *top = p_cur->hi_child.get();
          ++top;
        }
      }
    }
  }
}

}  // namespace ballistae

#endif
