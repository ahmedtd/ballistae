#ifndef LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH

////////////////////////////////////////////////////////////////////////////////
/// The interface that material plugins should implement.
////////////////////////////////////////////////////////////////////////////////

#include <random>

#include "libballistae/contact.hh"
#include "libballistae/ray.hh"

namespace ballistae {

template <class Field>
struct shade_info {
  Field propagation_k;
  Field emitted_power;
  ray<Field, 3> incident_ray;
};

class material {
 public:
  virtual ~material() {}

  virtual void crush(double time) = 0;

  virtual shade_info<double> shade(const contact<double> &glb_contact,
                                   double lambda) const = 0;
};

}  // namespace ballistae

#endif
