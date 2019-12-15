#ifndef LIBBALLISTAE_GEOM_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_GEOM_PLUGIN_INTERFACE_HH

#include <cstddef>
#include <vector>

#include "frustum/geometry/affine_transform.hh"
#include "libballistae/aabox.hh"
#include "libballistae/contact.hh"
#include "libballistae/ray.hh"
#include "libballistae/span.hh"

namespace ballistae {

class geometry {
 public:
  virtual ~geometry() {}

  virtual aabox<double, 3> get_aabox() = 0;

  virtual void crush(double time) = 0;

  virtual contact<double> ray_into(
      const ray_segment<double, 3> &query) const = 0;

  virtual contact<double> ray_exit(
      const ray_segment<double, 3> &query) const = 0;
};

}  // namespace ballistae

#endif
