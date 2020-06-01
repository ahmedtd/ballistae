#ifndef LIBBALLISTAE_MATERIAL_NORMAL_HIGHLIGHTER_HH
#define LIBBALLISTAE_MATERIAL_NORMAL_HIGHLIGHTER_HH

#include <random>

#include "libballistae/material.hh"

namespace ballistae {

namespace materials {

template <class EmissionFn>
class normal_highlighter : public material {
 public:
  fixvec<double, 3> highlight_direction;
  EmissionFn emission;

  normal_highlighter(fixvec<double, 3> highlight_direction_in,
                     EmissionFn emission_in)
      : highlight_direction(normalise(highlight_direction_in)),
        emission(emission_in) {}

  virtual ~normal_highlighter() {}

  virtual void crush(double time) {}

  virtual shade_info<double> shade(const contact<double> &glb_contact,
                                   float lambda, std::mt19937 &rng) const {
    shade_info<double> result;
    result.emitted_power =
        float(iprod(highlight_direction, glb_contact.n)) *
        emission({glb_contact.mtl2, glb_contact.mtl3, lambda});
    result.propagation_k = 0;

    return result;
  }
};

template <class EmissionFn>
auto make_normal_highlighter(fixvec<double, 3> highlight_direction,
                             EmissionFn emission) {
  return normal_highlighter<EmissionFn>(highlight_direction, emission);
}

}  // namespace materials

}  // namespace ballistae

#endif
