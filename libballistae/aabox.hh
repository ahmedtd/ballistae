#ifndef LIBBALLISTAE_AABOX_HH
#define LIBBALLISTAE_AABOX_HH

#include <array>
#include <cmath>
#include <utility>

#include "libballistae/ray.hh"
#include "libballistae/span.hh"
#include "libballistae/vector.hh"

namespace ballistae {

struct aabox final {
  std::array<span<double>, 3> spans;

  span<double> &operator[](size_t i) { return spans[i]; }

  const span<double> &operator[](size_t i) const { return spans[i]; }

  static aabox nan();

  // A bounding box suitable for use as the zero element in a call to
  // std::accumulate with the min_containing aggregator function.
  static aabox accum_zero();
};

inline aabox aabox::nan() {
  aabox result;
  for (size_t i = 0; i < 3; ++i) result.spans[i] = span<double>::nan();
  return result;
}

inline aabox aabox::accum_zero() {
  aabox result;
  for (size_t i = 0; i < 3; ++i) {
    result[i].lo = std::numeric_limits<double>::infinity();
    result[i].hi = -std::numeric_limits<double>::infinity();
  }
  return result;
}

/// Order A and B along the given axis.
///
/// Convenient for use with std::bind.
inline bool aabox_axial_comparator(size_t axis, const aabox &a,
                                   const aabox &b) {
  return a.spans[axis] < b.spans[axis];
}

/// Smallest aabox containing A and B.
inline aabox min_containing(const aabox &a, const aabox &b) {
  aabox result;
  for (size_t i = 0; i < 3; ++i)
    result.spans[i] = min_containing(a.spans[i], b.spans[i]);
  return result;
}

/// Grow BOX to include P.
inline aabox min_containing(const aabox &box, const fixvec<double, 3> &p) {
  aabox result;
  for (size_t i = 0; i < 3; ++i) result.spans[i] = min_containing(box[i], p(i));
  return result;
}

/// Cut BOX along AXIS at CUT_PLANE.
///
/// Preconditions:
///
///   * The plane defined by AXIS and CUT_PLANE must make contact with BOX.
inline std::array<aabox, 2> cut(const aabox &box, size_t axis,
                                const double &cut_plane) {
  auto span_cut = cut(box.spans[axis], cut_plane);

  aabox lo = box;
  aabox hi = box;
  lo.spans[axis] = span_cut[0];
  hi.spans[axis] = span_cut[1];
  return {lo, hi};
}

inline span<double> ray_test(const ray_segment &r, const aabox &b) {
  using std::swap;

  span<double> cover = {-std::numeric_limits<double>::infinity(),
                        std::numeric_limits<double>::infinity()};

  for (size_t i = 0; i < 3; ++i) {
    span<double> cur = {
        (b.spans[i].lo - r.the_ray.point(i)) / r.the_ray.slope(i),
        (b.spans[i].hi - r.the_ray.point(i)) / r.the_ray.slope(i)};

    if (cur.hi < cur.lo) {
      swap(cur.lo, cur.hi);
    }

    if (!(overlaps(cover, cur))) {
      return span<double>::nan();
    }

    cover = max_intersecting(cover, cur);
  }

  return overlaps(cover, r.the_segment) ? max_intersecting(cover, r.the_segment)
                                        : span<double>::nan();
}

inline double surface_area(const aabox &box) {
  double accum = 0;
  for (size_t axis_a = 0; axis_a < 3; ++axis_a) {
    for (size_t axis_b = axis_a; axis_b < 3; ++axis_b) {
      accum +=
          double(2) * measure(box.spans[axis_a]) * measure(box.spans[axis_b]);
    }
  }

  return accum;
}

inline bool isfinite(const aabox &box) {
  using std::isfinite;

  for (size_t i = 0; i < 3; ++i) {
    if (!isfinite(measure(box.spans[i]))) return false;
  }

  return true;
}

/// Get a bounding box that contains a transformed bounding box.
///
/// Note that incremental updates using these grown bounding boxes could cause
/// the bounding box to grow without bound.
inline aabox operator*(const affine_transform<double, 3> &t, const aabox &box) {
  auto result = aabox::accum_zero();

  size_t npoints = size_t(1) << 3;
  for (size_t i = 0; i < npoints; ++i) {
    fixvec<double, 3> cur_point;
    for (size_t j = 0; j < 3; ++j)
      cur_point(j) = ((i >> j) & 0x1) ? box[j].hi : box[j].lo;

    result = min_containing(result, t * cur_point);
  }

  return result;
}

}  // namespace ballistae

#endif
