#ifndef LIBBALLISTAE_MATERIAL_PC_SMOOTH_HH
#define LIBBALLISTAE_MATERIAL_PC_SMOOTH_HH

#include "include/libballistae/material.hh"

namespace ballistae
{

namespace materials
{

template<class ReflectanceFn>
class pc_smooth : public material
{
public:
    ReflectanceFn reflectance;

    pc_smooth(ReflectanceFn reflectance_in)
        : reflectance(reflectance_in)
    {
    }

    virtual ~pc_smooth()
    {
    }

    virtual void crush(double time)
    {
    }

    virtual shade_info<double> shade(
        const contact<double> &glb_contact,
        double lambda
    ) const
    {
        shade_info<double> result;
        result.emitted_power = 0.0;
        result.propagation_k = reflectance({glb_contact.mtl2, glb_contact.mtl3, lambda});
        result.incident_ray.point = glb_contact.p;
        result.incident_ray.slope = reflect(glb_contact.r.slope, glb_contact.n);

        return result;
    }
};

template<class ReflectanceFn>
auto make_pc_smooth(ReflectanceFn reflectance)
{
    return pc_smooth<ReflectanceFn>(reflectance);
}

}

}

#endif
