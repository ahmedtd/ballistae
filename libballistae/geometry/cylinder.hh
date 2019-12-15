#ifndef BALLISTAE_GEOMETRY_CYLINDER_HH
#define BALLISTAE_GEOMETRY_CYLINDER_HH

#include <frustum-0/indicial/fixed.hh>
#include <libballistae/contact.hh>
#include <libballistae/geometry.hh>
#include <libballistae/ray.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

namespace ballistae {

class cylinder : public geometry {
 public:
  fixvec<double, 3> center;
  fixvec<double, 3> axis;
  double radius_squared;

 public:
  cylinder(const fixvec<double, 3> &center_in, const fixvec<double, 3> &axis_in,
           const double &radius_in)
      : center(center_in),
        axis(normalise(axis_in)),
        radius_squared(radius_in * radius_in) {}

  virtual ~cylinder() {}

  virtual aabox<double, 3> get_aabox() {
    // Only a few infinite cylinders are bounded on any of the principle axes.

    aabox<double, 3> infinity = {-std::numeric_limits<double>::infinity(),
                                 std::numeric_limits<double>::infinity(),
                                 -std::numeric_limits<double>::infinity(),
                                 std::numeric_limits<double>::infinity(),
                                 -std::numeric_limits<double>::infinity(),
                                 std::numeric_limits<double>::infinity()};

    return infinity;
  }

  virtual void crush(double time) {}

  virtual contact<double> ray_into(const ray_segment<double, 3> &query) const {
    using std::atan2;
    using std::sqrt;

    auto foil_a = reject(axis, query.the_ray.slope);
    auto foil_b = reject(axis, query.the_ray.point - center);

    double a = iprod(foil_a, foil_a);
    double b = 2.0 * iprod(foil_a, foil_b);
    double c = iprod(foil_b, foil_b) - radius_squared;

    double t_min = (-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a);

    if (!std::isnan(t_min)) {
      // Ray hits cylinder.

      if (contains(query.the_segment, t_min)) {
        contact<double> result;

        result.t = t_min;
        result.r = query.the_ray;
        result.p = eval_ray(query.the_ray, t_min);
        result.n = reject(axis, result.p - center);
        result.mtl2 = {result.p(0), std::atan2(result.p(1), result.p(2))};
        result.mtl3 = result.p;

        return result;
      } else {
        return contact<double>::nan();
      }
    } else {
      // Ray misses cylinder.  It could miss outside or inside.

      if (c > double(0)) {
        return ballistae::contact<double>::nan();
      } else {
        if (contains(query.the_segment,
                     -std::numeric_limits<double>::infinity())) {
          contact<double> result;
          result.t = -std::numeric_limits<double>::infinity();
          result.r = query.the_ray;
          return result;
        } else
          return contact<double>::nan();
      }
    }
  }

  virtual contact<double> ray_exit(const ray_segment<double, 3> &query) const {
    using std::atan2;
    using std::sqrt;

    auto foil_a = reject(axis, query.the_ray.slope);
    auto foil_b = reject(axis, query.the_ray.point - center);

    double a = iprod(foil_a, foil_a);
    double b = 2.0 * iprod(foil_a, foil_b);
    double c = iprod(foil_b, foil_b) - radius_squared;

    double t_max = (-b + sqrt(b * b - 4.0 * a * c)) / (2.0 * a);

    if (!std::isnan(t_max)) {
      // The ray hits the cylinder.

      if (contains(query.the_segment, t_max)) {
        contact<double> result;

        result.t = t_max;
        result.r = query.the_ray;
        result.p = eval_ray(query.the_ray, t_max);
        result.n = reject(axis, result.p - center);
        result.mtl2 = {result.p(0), std::atan2(result.p(1), result.p(2))};
        result.mtl3 = result.p;

        return result;
      } else {
        return contact<double>::nan();
      }
    } else {
      // Ray misses cylinder.  It could miss inside or outside.

      if (contains(query.the_segment,
                   std::numeric_limits<double>::infinity())) {
        contact<double> result;
        result.t = std::numeric_limits<double>::infinity();
        result.r = query.the_ray;
        return result;
      } else
        return contact<double>::nan();
    }
  }
};

}  // namespace ballistae

#endif
