#ifndef BALLISTAE_MATERIAL_EMITTER_HH
#define BALLISTAE_MATERIAL_EMITTER_HH

#include <random>

#include "frustum/indicial/fixed.hh"
#include "libballistae/dense_signal.hh"
#include "libballistae/material.hh"
#include "libballistae/material_map.hh"

namespace ballistae {

namespace materials {

/// An emitter that simply feeds geometric material coordinates into
/// an emissivity material map.
template <class EmissivityFn>
class emitter : public material {
 public:
  EmissivityFn emissivity;

 public:
  emitter(EmissivityFn emissivity_in) : emissivity(emissivity_in) {}

  virtual ~emitter() {}

  virtual void crush(double time) {}

  virtual shade_info<double> shade(const contact<double> &glb_contact,
                                   double lambda, std::mt19937 &rng) const {
    const auto &mtl2 = glb_contact.mtl2;
    const auto &mtl3 = glb_contact.mtl3;

    shade_info<double> result;
    result.propagation_k = 0.0;
    result.emitted_power = emissivity({mtl2, mtl3, lambda});

    return result;
  }
};

template <class EmissivityFn>
auto make_emitter(EmissivityFn emissivity) {
  return emitter<EmissivityFn>(emissivity);
}

}  // namespace materials

}  // namespace ballistae

#endif
