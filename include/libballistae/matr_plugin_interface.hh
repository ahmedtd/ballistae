#ifndef LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH

////////////////////////////////////////////////////////////////////////////////
/// The interface that material plugins should implement.
////////////////////////////////////////////////////////////////////////////////

#include <libballistae/ray.hh>
#include <libballistae/span.hh>
#include <libballistae/spectrum.hh>

namespace ballistae
{

class matr_priv
{
public:

    virtual ~matr_priv() {}

    virtual ballistae::color_d_rgb shade(
        const ballistae::dray3 &eye_ray,
        const ballistae::span<double> &span,
        const arma::vec3 *span_normals
    ) const = 0;

};

}

#endif
