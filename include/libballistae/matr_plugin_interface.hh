#ifndef LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH

////////////////////////////////////////////////////////////////////////////////
/// The interface that material plugins should implement.
////////////////////////////////////////////////////////////////////////////////

#include <random>

#include <armadillo>

#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/spectrum.hh>

namespace ballistae
{

template<class Field>
struct shade_info
{
    Field propagation_k;
    Field emitted_power;
    ray<Field, 3> incident_ray;
};

class matr_priv
{
public:

    virtual ~matr_priv() {}

    virtual shade_info<double> shade(
        const ballistae::scene &the_scene,
        const ballistae::dray3 &reflected_ray,
        const ballistae::span<double> &contact_span,
        double lambda_nm,
        std::mt19937 &thread_rng
    ) const = 0;

};

}

#endif
