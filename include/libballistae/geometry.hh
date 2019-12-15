#ifndef LIBBALLISTAE_GEOM_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_GEOM_PLUGIN_INTERFACE_HH

#include <cstddef>

#include <vector>

#include "include/frustum-0/geometry/affine_transform.hh"

#include "include/libballistae/aabox.hh"
#include "include/libballistae/contact.hh"
#include "include/libballistae/ray.hh"
#include "include/libballistae/span.hh"

namespace ballistae
{

class geometry
{
public:
    virtual ~geometry() {}

    virtual aabox<double, 3> get_aabox() = 0;

    virtual void crush(double time) = 0;

    virtual contact<double> ray_into(
        const ray_segment<double, 3> &query
    ) const = 0;

    virtual contact<double> ray_exit(
        const ray_segment<double, 3> &query
    ) const = 0;
};

}

#endif
