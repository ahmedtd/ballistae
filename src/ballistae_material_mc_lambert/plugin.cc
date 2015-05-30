#include <libballistae/material.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

#include <frustum-0/indicial/fixed.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

#include <libballistae/material_map.hh>
#include <libballistae/dense_signal.hh>
#include <libballistae/vector_distributions.hh>

using namespace frustum;
using namespace ballistae;

class mc_lambert : public ballistae_guile::updatable_material
{
public:

    size_t reflectance_ind;
    mtlmap<1> *reflectance;

    mc_lambert();
    virtual ~mc_lambert();

    virtual void crush(const scene &the_scene, double time);

    virtual shade_info<double> shade(
        const scene &the_scene,
        const contact<double> &glb_contact,
        double lambda
    ) const;

    virtual void guile_update(scene *p_scene, SCM config);
};

mc_lambert::mc_lambert()
{
}

mc_lambert::~mc_lambert()
{
}

void mc_lambert::crush(const scene &the_scene, double time)
{
    reflectance = the_scene.mtlmaps_1[reflectance_ind].get();
    reflectance->crush(the_scene, time);
}

shade_info<double> mc_lambert::shade(
    const scene &the_scene,
    const contact<double> &glb_contact,
    double lambda
) const
{
    static thread_local std::ranlux24 thread_rng;

    shade_info<double> result;

    // Monte-carlo part.

    hemisphere_unitv_distribution<double, 3> dist(glb_contact.n);
    auto dir = dist(thread_rng);

    result.incident_ray.point = glb_contact.p;
    result.incident_ray.slope = dir;

    double r = reflectance->value(glb_contact.mtl2, glb_contact.mtl3, lambda)(0);
    result.propagation_k = iprod(glb_contact.n, dir) * r;

    // Standard illuminator part.

    result.emitted_power = 0.0;

    for(const auto& illum : the_scene.illuminators)
    {
        // illum is a unique_pointer.
        auto illum_info = illum->power_at_point(
            the_scene,
            glb_contact.p,
            lambda
        );

        double cosine = -iprod(illum_info.arrival, glb_contact.n);
        cosine = (cosine < 0.0) ? 0.0 : cosine;
        result.emitted_power += illum_info.power * cosine;
    }

    result.emitted_power *= r;

    return result;
}

void mc_lambert::guile_update(scene *p_scene, SCM config)
{
    SCM sym_reflectance = scm_from_utf8_symbol("reflectance");
    SCM lu_reflectance = scm_assq_ref(config, sym_reflectance);

    if(scm_is_true(lu_reflectance))
        this->reflectance_ind = scm_to_size_t(lu_reflectance);
}

std::unique_ptr<ballistae_guile::updatable_material>
guile_ballistae_material(scene *p_scene, SCM config)
{
    using namespace ballistae_guile;

    auto p = std::make_unique<mc_lambert>();

    // mtlmap 0 is *always* a constant smits_white mtlmap.
    p->reflectance_ind = 0;
    p->guile_update(p_scene, config);

    return std::move(p);
}
