#ifndef LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH

////////////////////////////////////////////////////////////////////////////////
/// The interface that material plugins should implement.
////////////////////////////////////////////////////////////////////////////////

#include <random>

#include "libballistae/contact.hh"
#include "libballistae/ray.hh"

namespace ballistae {

struct shade_info {
  float propagation_k;
  float emitted_power;
  ray incident_ray;
};

class material {
 public:
  virtual ~material() {}

  virtual void crush(double time) = 0;

  virtual shade_info shade(const contact &glb_contact, float lambda,
                           std::mt19937 &thread_rng) const = 0;
};

}  // namespace ballistae

#endif
