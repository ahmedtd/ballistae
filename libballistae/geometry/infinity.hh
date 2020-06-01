#ifndef BALLISTAE_GEOMETRY_INFINITY_HH
#define BALLISTAE_GEOMETRY_INFINITY_HH

#include "libballistae/geometry.hh"
#include "libballistae/ray.hh"
#include "libballistae/span.hh"
#include "libballistae/vector.hh"

namespace ballistae {

class infinity final : public geometry {
 public:
  virtual aabox get_aabox() {
    // What were you expecting?
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
    using std::acos;
    using std::atan2;

    auto infty = std::numeric_limits<double>::infinity();

    if (query.the_segment.hi == infty) {
      fixvec<double, 3> p = eval_ray(query.the_ray, infty);
      contact result = {infty,
                        query.the_ray,
                        p,
                        -query.the_ray.slope,
                        {atan2(query.the_ray.slope(0), query.the_ray.slope(1)),
                         acos(query.the_ray.slope(2))},
                        p};
      return result;
    } else {
      return contact::nan();
    }
  }

  virtual contact ray_exit(const ray_segment &query) const {
    using std::acos;
    using std::atan2;

    auto infty = std::numeric_limits<double>::infinity();

    if (query.the_segment.lo == -infty) {
      fixvec<double, 3> p = eval_ray(query.the_ray, infty);
      contact result = {
          -infty,
          query.the_ray,
          p,
          query.the_ray.slope,
          {atan2(-query.the_ray.slope(0), -query.the_ray.slope(1)),
           acos(-query.the_ray.slope(2))},
          p};
      return result;
    } else {
      return contact::nan();
    }
  }
};

}  // namespace ballistae

#endif
