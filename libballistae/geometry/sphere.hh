#ifndef BALLISTAE_GEOMETRY_SPHERE_HH
#define BALLISTAE_GEOMETRY_SPHERE_HH

#include <cmath>
#include <random>

#include "libballistae/geometry.hh"
#include "libballistae/ray.hh"
#include "libballistae/span.hh"
#include "libballistae/vector.hh"

namespace ballistae {

class sphere : public geometry {
 public:
  sphere() {}

  virtual ~sphere() {}

  virtual aabox<double, 3> get_aabox() {
    aabox<double, 3> infinity = {-1.0, 1.0, -1.0, 1.0, -1.0, 1.0};

    return infinity;
  }

  virtual void crush(double time) {
    // Nothing to do.
  }

  virtual contact<double> ray_into(const ray_segment<double, 3> &query) const {
    using std::acos;
    using std::atan2;
    using std::sqrt;

    auto b = iprod(query.the_ray.slope, query.the_ray.point);
    auto c = iprod(query.the_ray.point, query.the_ray.point) - 1.0;

    // We rely on std::sqrt's mandated NaN behavior.
    auto t_min = -b - sqrt(b * b - c);

    if (contains(query.the_segment, t_min)) {
      contact<double> result;

      auto p = eval_ray(query.the_ray, t_min);
      result.t = t_min;
      result.p = p;
      result.n = normalise(p);
      result.mtl2 = {atan2(p(0), p(1)), acos(p(2))};
      result.mtl3 = p;
      result.r = query.the_ray;

      return result;
    } else {
      return contact<double>::nan();
    }
  }

  virtual contact<double> ray_exit(const ray_segment<double, 3> &query) const {
    using std::asin;
    using std::atan2;
    using std::sqrt;

    auto b = iprod(query.the_ray.slope, query.the_ray.point);
    auto c = iprod(query.the_ray.point, query.the_ray.point) - 1.0;

    // We rely on std::sqrt's mandated NaN behavior.
    auto t_max = -b + sqrt(b * b - c);

    if (contains(query.the_segment, t_max)) {
      contact<double> result;
      auto p = eval_ray(query.the_ray, t_max);
      result.t = t_max;
      result.p = p;
      result.n = normalise(p);
      result.mtl2 = {atan2(p(0), p(1)), asin(p(2))};
      result.mtl3 = p;
      result.r = query.the_ray;

      return result;
    } else {
      return contact<double>::nan();
    }
  }
};

}  // namespace ballistae

#endif
