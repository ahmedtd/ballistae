#ifndef LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH

////////////////////////////////////////////////////////////////////////////////
/// The interface that material plugins should implement.
////////////////////////////////////////////////////////////////////////////////

#include <random>

#include <armadillo>

#include <libballistae/contact.hh>
#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
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
        const scene &the_scene,
        const contact<double> &glb_contact,
        double lambda_nm,
        size_t sample_index,
        std::ranlux24 &thread_rng
    ) const = 0;

};

}

#endif
