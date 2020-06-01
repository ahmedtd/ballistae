#ifndef BALLISTAE_GEOMETRY_PLANE_HH
#define BALLISTAE_GEOMETRY_PLANE_HH

#include "frustum/indicial/fixed.hh"
#include "libballistae/contact.hh"
#include "libballistae/geometry.hh"
#include "libballistae/ray.hh"
#include "libballistae/span.hh"

namespace ballistae {

class plane : public geometry {
 public:
  plane() {}

  virtual ~plane() {}

  virtual aabox get_aabox() {
    // Very few planes are not infinite on all principle axes.
    aabox infinity = {-std::numeric_limits<double>::infinity(),
                      std::numeric_limits<double>::infinity(),
                      -std::numeric_limits<double>::infinity(),
                      std::numeric_limits<double>::infinity(),
                      -std::numeric_limits<double>::infinity(),
                      std::numeric_limits<double>::infinity()};

    return infinity;
  }

  virtual void crush(double time) {}

  virtual contact ray_into(const ray_segment &query) const {
    auto height = query.the_ray.point(0);
    auto slope = query.the_ray.slope(0);
    auto t = -height / slope;

    if (slope < double(0) && contains(query.the_segment, t)) {
      contact result;

      result.t = t;
      result.r = query.the_ray;
      result.p = eval_ray(query.the_ray, t);
      result.n = {1, 0, 0};
      result.mtl2 = {result.p(1), result.p(2)};
      result.mtl3 = result.p;

      return result;
    } else {
      // Ray never intersects plane.
      return contact::nan();
    }
  }

  virtual contact ray_exit(const ray_segment &query) const {
    auto height = query.the_ray.point(0);
    auto slope = query.the_ray.slope(0);
    auto t = -height / slope;

    if (slope > double(0) && contains(query.the_segment, t)) {
      contact result;

      result.t = t;
      result.r = query.the_ray;
      result.p = eval_ray(query.the_ray, t);
      result.n = {1, 0, 0};
      result.mtl2 = {result.p(1), result.p(2)};
      result.mtl3 = result.p;

      return result;
    } else {
      // Ray never intersects plane.
      return contact::nan();
    }
  }
};

}  // namespace ballistae

#endif
