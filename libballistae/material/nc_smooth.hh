#ifndef BALLISTAE_MATERIAL_NC_SMOOTH_HH
#define BALLISTAE_MATERIAL_NC_SMOOTH_HH

#include <random>

#include "frustum/indicial/fixed.hh"
#include "libballistae/dense_signal.hh"
#include "libballistae/material.hh"
#include "libballistae/material_map.hh"
#include "libballistae/vector.hh"

namespace ballistae {

namespace materials {

template <class NInteriorFn, class NExteriorFn>
class nc_smooth : public material {
 public:
  NInteriorFn n_interior;
  NExteriorFn n_exterior;

 public:
  nc_smooth(NInteriorFn n_interior_in, NExteriorFn n_exterior_in)
      : n_interior(n_interior_in), n_exterior(n_exterior_in) {}

  virtual ~nc_smooth() {}

  virtual void crush(double time) {}

  virtual shade_info shade(const contact &glb_contact, double lambda,
                           std::mt19937 &rng) const {
    using std::pow;
    using std::sqrt;
    using std::swap;

    fixvec<double, 3> p = glb_contact.p;
    fixvec<double, 3> n = glb_contact.n;
    fixvec<double, 3> refl = glb_contact.r.slope;

    double a_cos = iprod(refl, n);

    double n_a =
        n_exterior(material_coords{glb_contact.mtl2, glb_contact.mtl3, lambda});
    double n_b =
        n_interior(material_coords{glb_contact.mtl2, glb_contact.mtl3, lambda});

    if (a_cos > 0.0) {
      swap(n_a, n_b);
    }

    double n_r = n_a / n_b;

    // Now the problem is regularized.  We have a ray that was emitted into
    // region A from a planar boundary.  There are two incident rays that put
    // power into this ray:
    //
    //   * One shone on the boundary from region A, and was partially reflected
    //     into the ray we have.
    //
    //   * One shone on the boundary from region B, and was partially
    //     transmitted into the ray we have.

    double snell = 1 - pow(n_r, 2) * (1.0 - pow(a_cos, 2));

    if (snell < 0.0) {
      // All power was contributed by the reflected ray (total internal
      // reflection).

      shade_info result;
      result.emitted_power = 0;
      result.propagation_k = 1.0;
      result.incident_ray.point = p;
      result.incident_ray.slope = reflect(refl, n);
      return result;
    }

    double b_cos = (a_cos < 0.0) ? -sqrt(snell) : sqrt(snell);

    double ab = n_r * b_cos / a_cos;
    double ab_i = 1.0 / ab;

    // We are solving the reverse problem, so the coefficient of transmission
    // must actually be calculated from the perspective of the ray shining on
    // the boundary from region B.
    double coeff_refl = pow((1 - ab) / (1 + ab), 2);
    double coeff_tran = ab_i * pow(2 / (1 + ab_i), 2);

    std::uniform_real_distribution<> dist(0, coeff_refl + coeff_tran);

    shade_info result;
    result.emitted_power = 0.0;
    if (dist(rng) < coeff_refl) {
      // Give the ray that contributed by reflection.
      result.propagation_k = 1;
      result.incident_ray.slope = reflect(refl, n);
    } else {
      // Give the ray that contributed by refraction.
      result.propagation_k = 1;
      result.incident_ray.slope = (b_cos - n_r * a_cos) * n + n_r * refl;
    }

    result.incident_ray.point = p;

    return result;
  }
};

template <class NInteriorFn, class NExteriorFn>
auto make_nc_smooth(NInteriorFn n_interior, NExteriorFn n_exterior) {
  return nc_smooth<NInteriorFn, NExteriorFn>(n_interior, n_exterior);
}

}  // namespace materials

}  // namespace ballistae

#endif
