#ifndef LIBBALLISTAE_RAY_HH
#define LIBBALLISTAE_RAY_HH

#include "frustum/geometry/affine_transform.hh"
#include "libballistae/span.hh"
#include "libballistae/vector.hh"

namespace ballistae {

/// A ray, emanating from POINT in direction SLOPE.
///
/// Invariants:
///
///   * (norm(slope) == 1): SLOPE is a unit vector.
struct ray {
  fixvec<double, 3> point;
  fixvec<double, 3> slope;
};

/// Evaluate R for the given parameter value T.
inline fixvec<double, 3> eval_ray(const ray &r, const double &t) {
  return r.point + t * r.slope;
}

inline ray operator*(const affine_transform<double, 3> &a, const ray &b) {
  return {a.linear * b.point + a.offset, normalise(a.linear * b.slope)};
}

struct ray_segment {
  ray the_ray;
  span<double> the_segment;
};

inline ray_segment operator*(const affine_transform<double, 3> &a,
                             const ray_segment &b) {
  ray_segment result;

  result.the_ray.point = a * b.the_ray.point;
  result.the_ray.slope = a.linear * b.the_ray.slope;

  // Assume that the input ray has unit slope.
  double scale_factor = norm(result.the_ray.slope);
  result.the_ray.slope /= scale_factor;

  result.the_segment = scale_factor * b.the_segment;

  return result;
}

}  // namespace ballistae

#endif
