#ifndef LIBBALLISTAE_CONTACT_HH
#define LIBBALLISTAE_CONTACT_HH

#include "frustum/geometry/affine_transform.hh"
#include "libballistae/ray.hh"
#include "libballistae/vector.hh"

namespace ballistae {

struct contact final {
  double t;
  ray r;
  fixvec<double, 3> p;
  fixvec<double, 3> n;
  fixvec<double, 2> mtl2;
  fixvec<double, 3> mtl3;

  inline static contact nan() {
    constexpr double nan = std::numeric_limits<double>::quiet_NaN();
    contact result;
    result.t = nan;
    return result;
  }
};

/// Apply an affine transform to a contact.
///
/// We require the transpose inverse of the linear part of the transform.
/// Rather than calculate it every time, we take it as an argument.
inline contact contact_transform(const contact &c,
                                 const affine_transform<double, 3> &t,
                                 const fixmat<double, 3, 3> &nm) {
  contact result = c;

  // We don't use the standard ray-transforming support, since we
  // need to know how the transform changes the scale of the
  // underlying space.
  result.r.point = t * result.r.point;
  result.r.slope = t.linear * result.r.slope;

  // Assume that the incoming ray has normalized slope.
  double scale_factor = norm(result.r.slope);
  result.r.slope /= scale_factor;
  result.t *= scale_factor;

  result.p = t * result.p;
  result.n = normalise(nm * result.n);
  return result;
}

}  // namespace ballistae

#endif
