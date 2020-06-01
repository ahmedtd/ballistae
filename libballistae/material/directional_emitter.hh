#ifndef BALLISTAE_MATERIAL_DIRECTIONAL_EMITTER_HH
#define BALLISTAE_MATERIAL_DIRECTIONAL_EMITTER_HH

#include <frustum-0/indicial/fixed.hh>
#include <libballistae/dense_signal.hh>
#include <libballistae/material.hh>
#include <libballistae/material_map.hh>
#include <random>

namespace ballistae {

namespace materials {

/// An emitter that queries an emissivity material map based on direction of
/// arrival.  In effect, it acts as a window into an environment map.
template <class EmissivityFn>
class directional_emitter : public material {
 public:
  EmissivityFn emissivity;

  directional_emitter(EmissivityFn emissivity_in) : emissivity(emissivity_in) {}

  virtual ~directional_emitter() {}

  virtual void crush(double time) {}

  virtual shade_info shade(const contact &glb_contact, float lambda_cur,
                           std::mt19937 &rng) const {
    using std::max;

    const auto &r = glb_contact.r.slope;
    fixvec<double, 3> mtl3 = r;
    fixvec<double, 2> mtl2 = {atan2(r(0), r(1)), acos(r(2))};

    shade_info result;
    result.propagation_k = 0.0f;
    result.emitted_power = emissivity({mtl2, mtl3, lambda});

    return result;
  }
};

template <class EmissivityFn>
auto make_directional_emitter(EmissivityFn emissivity) {
  return directional_emitter<EmissivityFn>(emissivity);
}

}  // namespace materials

}  // namespace ballistae

#endif
