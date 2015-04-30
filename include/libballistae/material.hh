#ifndef LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_MATR_PLUGIN_INTERFACE_HH

////////////////////////////////////////////////////////////////////////////////
/// The interface that material plugins should implement.
////////////////////////////////////////////////////////////////////////////////

#include <random>

#include <libballistae/contact.hh>
#include <libballistae/ray.hh>
#include <libballistae/scene.hh>

namespace ballistae
{

template<class Field>
struct shade_info
{
    Field propagation_k;
    Field emitted_power;
    ray<Field, 3> incident_ray;
};

class material
{
public:

    virtual ~material() {}

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
