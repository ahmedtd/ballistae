#include <libballistae/material.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/material_map.hh>
#include <libballistae/scene.hh>
#include <libballistae/vector.hh>

using namespace frustum;
using namespace ballistae;

class pc_smooth : public ballistae_guile::updatable_material
{
public:
    const mtlmap<1> *reflectance;

    virtual ~pc_smooth();

    virtual shade_info<double> shade(
        const scene &the_scene,
        const contact<double> &glb_contact,
        double lambda_src,
        double lambda_lim,
        double lambda_cur,
        size_t sample_index,
        std::ranlux24 &thread_rng
    ) const;

    virtual void guile_update(scene *p_scene, SCM config);
};

pc_smooth::~pc_smooth()
{
}

void pc_smooth::guile_update(scene *p_scene, SCM config)
{
    using namespace ballistae_guile;

    SCM sym_reflectance = scm_from_utf8_symbol("reflectance");
    SCM lu_reflectance = scm_assq_ref(config, sym_reflectance);

    if(scm_is_true(lu_reflectance))
        this->reflectance = p_scene->mtlmaps_1[scm_to_size_t(lu_reflectance)].get();
}

shade_info<double> pc_smooth::shade(
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
    result.emitted_power = 0.0;
    result.propagation_k = reflectance->value(glb_contact.mtl2, glb_contact.mtl3, lambda_cur)(0);
    result.incident_ray.point = glb_contact.p;
    result.incident_ray.slope = reflect(glb_contact.r.slope, glb_contact.n);

    return result;
}

ballistae_guile::updatable_material*
guile_ballistae_material(scene *p_scene, SCM config)
{
    pc_smooth *p = new pc_smooth();

    p->reflectance = p_scene->mtlmaps_1[0].get();

    p->guile_update(p_scene, config);

    return p;
}
