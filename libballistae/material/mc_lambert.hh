#ifndef BALLISTAE_MATERIAL_MC_LAMBERT_HH
#define BALLISTAE_MATERIAL_MC_LAMBERT_HH

#include <random>

#include "frustum/indicial/fixed.hh"
#include "libballistae/dense_signal.hh"
#include "libballistae/material.hh"
#include "libballistae/material_map.hh"
#include "libballistae/vector_distributions.hh"

namespace ballistae {

namespace materials {

template <class ReflectanceFn>
class mc_lambert : public material {
 public:
  ReflectanceFn reflectance;

  mc_lambert(ReflectanceFn reflectance_in) : reflectance(reflectance_in) {}

  virtual ~mc_lambert() {}

  virtual void crush(double time) {}

  virtual shade_info shade(const contact &glb_contact, double lambda,
                           std::mt19937 &rng) const {
    static thread_local std::mt19937 thread_rng;

    shade_info result;

    hemisphere_unitv_distribution<double, 3> dist(glb_contact.n);
    auto dir = dist(thread_rng);

    result.incident_ray.point = glb_contact.p;
    result.incident_ray.slope = dir;

    double r = reflectance({glb_contact.mtl2, glb_contact.mtl3, lambda});
    result.propagation_k = iprod(glb_contact.n, dir) * r;

    result.emitted_power = 0.0;

    return result;
  }
};

template <class ReflectanceFn>
auto make_mc_lambert(ReflectanceFn reflectance) {
  return mc_lambert<ReflectanceFn>(reflectance);
}

}  // namespace materials

}  // namespace ballistae

#endif
