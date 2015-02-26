#ifndef LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH

////////////////////////////////////////////////////////////////////////////////
/// The interface that material plugins should implement.
////////////////////////////////////////////////////////////////////////////////

#include <armadillo>

#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/spectrum.hh>

namespace ballistae
{

class matr_priv
{
public:

    virtual ~matr_priv() {}

    virtual void shade(
        const ballistae::scene &the_scene,
        const ballistae::dray3 *eyes_src,
        const ballistae::dray3 *eyes_lim,
        const ballistae::span<double> *spans_src,
        const arma::vec3 *normals_src,
        ballistae::color_d_rgb *shades_out_src
    ) const = 0;

};

}

#endif
