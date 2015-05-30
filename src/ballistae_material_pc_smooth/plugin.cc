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
    size_t reflectance_ind;
    mtlmap<1> *reflectance;

    virtual ~pc_smooth();

    virtual void crush(const scene &the_scene, double time);

    virtual shade_info<double> shade(
        const scene &the_scene,
        const contact<double> &glb_contact,
        double lambda
    ) const;

    virtual void guile_update(scene *p_scene, SCM config);
};

pc_smooth::~pc_smooth()
{
}

void pc_smooth::crush(const scene &the_scene, double time)
{
    reflectance = the_scene.mtlmaps_1[reflectance_ind].get();
    reflectance->crush(the_scene, time);
}

void pc_smooth::guile_update(scene *p_scene, SCM config)
{
    using namespace ballistae_guile;

    SCM sym_reflectance = scm_from_utf8_symbol("reflectance");
    SCM lu_reflectance = scm_assq_ref(config, sym_reflectance);

    if(scm_is_true(lu_reflectance))
        this->reflectance_ind = scm_to_size_t(lu_reflectance);
}

shade_info<double> pc_smooth::shade(
     const scene &the_scene,
    const contact<double> &glb_contact,
    double lambda
) const
{
    shade_info<double> result;
    result.emitted_power = 0.0;
    result.propagation_k = reflectance->value(glb_contact.mtl2, glb_contact.mtl3, lambda)(0);
    result.incident_ray.point = glb_contact.p;
    result.incident_ray.slope = reflect(glb_contact.r.slope, glb_contact.n);

    return result;
}

std::unique_ptr<ballistae_guile::updatable_material>
guile_ballistae_material(scene *p_scene, SCM config)
{
    auto p = std::make_unique<pc_smooth>();

    p->reflectance_ind = 0;

    p->guile_update(p_scene, config);

    return std::move(p);
}
