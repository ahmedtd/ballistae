#include <libballistae/material.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

#include <frustum-0/indicial/fixed.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

#include <libballistae/dense_signal.hh>
#include <libballistae/vector_distributions.hh>

using namespace frustum;
using namespace ballistae;

class mc_lambert : public material
{
public:

    dense_signal<double> reflectance;

    mc_lambert();
    virtual ~mc_lambert();

    virtual shade_info<double> shade(
        const scene &the_scene,
        const contact<double> &glb_contact,
        double lambda_src,
        double lambda_lim,
        double lambda_cur,
        size_t sample_index,
        std::ranlux24 &thread_rng
    ) const;
};

mc_lambert::mc_lambert()
{
}

mc_lambert::~mc_lambert()
{
}

shade_info<double> mc_lambert::shade(
    const scene &the_scene,
    const contact<double> &glb_contact,
    double lambda_src,
    double lambda_lim,
    double lambda_cur,
    size_t sample_index,
    std::ranlux24 &thread_rng
) const
{
    shade_info<double> result;
    result.emitted_power = 0;

    hemisphere_unitv_distribution<double, 3> dist(glb_contact.n);
    auto dir = dist(thread_rng);

    result.incident_ray.point = glb_contact.p;
    result.incident_ray.slope = dir;

    result.propagation_k =
        iprod(glb_contact.n, dir) * interpolate(reflectance, lambda_cur);

    return result;
}

material* guile_ballistae_material(SCM config)
{
    using namespace ballistae_guile;

    mc_lambert *p = new mc_lambert();

    p->reflectance = smits_white<double>();

    guile_ballistae_update_material(p, config);

    return p;
}

material* guile_ballistae_update_material(material *p_matr, SCM config)
{
    using namespace ballistae_guile;

    mc_lambert *p = dynamic_cast<mc_lambert*>(p_matr);

    SCM sym_reflectance = scm_from_utf8_symbol("reflectance");
    SCM lu_reflectance = scm_assq_ref(config, sym_reflectance);

    if(scm_is_true(lu_reflectance))
        p->reflectance = signal_from_scm(lu_reflectance);

    return p;
}
