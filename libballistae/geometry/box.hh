#ifndef BALLISTAE_GEOMETRY_BOX_HH
#define BALLISTAE_GEOMETRY_BOX_HH

#include <array>

#include "libballistae/contact.hh"
#include "libballistae/geometry.hh"
#include "libballistae/ray.hh"
#include "libballistae/span.hh"
#include "libballistae/vector.hh"

namespace ballistae {

class box : public geometry {
 public:
  std::array<span<double>, 3> spans;

  box(std::array<span<double>, 3> spans_in) : spans(spans_in) {}

  virtual ~box() {}

  virtual aabox<double, 3> get_aabox() { return aabox<double, 3>{spans}; }

  virtual void crush(double time) {}

  virtual contact<double> ray_into(const ray_segment<double, 3> &query) const {
    using std::max;
    using std::min;
    using std::swap;

    span<double> cover = {-std::numeric_limits<double>::infinity(),
                          std::numeric_limits<double>::infinity()};

    fixvec<double, 3> hit_axis = {0, 0, 0};
    for (size_t i = 0; i < 3; ++i) {
      span<double> cur = {
          (spans[i].lo - query.the_ray.point(i)) / query.the_ray.slope(i),
          (spans[i].hi - query.the_ray.point(i)) / query.the_ray.slope(i)};

      double normal_component = -1;
      if (cur.hi < cur.lo) {
        swap(cur.lo, cur.hi);
        normal_component = 1;
      }

      if (!(overlaps(cover, cur))) return contact<double>::nan();

      if (cover.lo < cur.lo) {
        cover.lo = cur.lo;
        hit_axis = {0, 0, 0};
        hit_axis[i] = normal_component;
      }

      if (cur.hi < cover.hi) {
        cover.hi = cur.hi;
      }
    }

    if (overlaps(cover, query.the_segment)) {
      contact<double> result;

      result.t = cover.lo;
      result.r = query.the_ray;
      result.p = eval_ray(query.the_ray, result.t);
      result.n = hit_axis;
      result.mtl2 = {0, 0};
      result.mtl3 = {result.p};

      return result;
    } else {
      return contact<double>::nan();
    }
  }

  virtual contact<double> ray_exit(const ray_segment<double, 3> &query) const {
    using std::max;
    using std::min;
    using std::swap;

    span<double> cover = {-std::numeric_limits<double>::infinity(),
                          std::numeric_limits<double>::infinity()};

    fixvec<double, 3> hit_axis = {0, 0, 0};
    for (size_t i = 0; i < 3; ++i) {
      span<double> cur = {
          (spans[i].lo - query.the_ray.point(i)) / query.the_ray.slope(i),
          (spans[i].hi - query.the_ray.point(i)) / query.the_ray.slope(i)};

      double normal_component = 1;
      if (cur.hi < cur.lo) {
        swap(cur.lo, cur.hi);
        normal_component = -1;
      }

      if (!(overlaps(cover, cur))) return contact<double>::nan();

      if (cover.lo < cur.lo) {
        cover.lo = cur.lo;
      }

      if (cur.hi < cover.hi) {
        cover.hi = cur.hi;
        hit_axis = {0, 0, 0};
        hit_axis[i] = normal_component;
      }
    }

    if (overlaps(cover, query.the_segment)) {
      contact<double> result;

      result.t = cover.hi;
      result.r = query.the_ray;
      result.p = eval_ray(query.the_ray, result.t);
      result.n = hit_axis;
      result.mtl2 = {0, 0};
      result.mtl3 = {result.p};

      return result;
    } else {
      return contact<double>::nan();
    }
  }
};

}  // namespace ballistae

#endif
