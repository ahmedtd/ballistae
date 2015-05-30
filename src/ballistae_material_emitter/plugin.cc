#include <libballistae/material.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/dense_signal.hh>
#include <libballistae/material_map.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

using namespace frustum;
using namespace ballistae;

/// An emitter that simply feeds geometric material coordinates into
/// an emissivity material map.
class emitter : public ballistae_guile::updatable_material
{
public:
    size_t emissivity_ind;
    mtlmap<1> *emissivity;

public:

    emitter();
    virtual ~emitter();

    virtual void crush(const scene &the_scene, double time);
    
    virtual shade_info<double> shade(
        const scene &the_scene,
        const contact<double> &glb_contact,
        double lambda_cur
    ) const;

    virtual void guile_update(scene *p_scene, SCM config);
};

emitter::emitter()
{
}

emitter::~emitter()
{
}

void emitter::crush(const scene &the_scene, double time)
{
    emissivity = the_scene.mtlmaps_1[emissivity_ind].get();
    emissivity->crush(the_scene, time);
}

void emitter::guile_update(scene *p_scene, SCM config)
{
    using namespace ballistae_guile;

    SCM sym_emissivity = scm_from_utf8_symbol("emissivity");
    SCM lu_emissivity = scm_assq_ref(config, sym_emissivity);

    if(scm_is_true(lu_emissivity))
        this->emissivity_ind = scm_to_size_t(lu_emissivity);
}

shade_info<double> emitter::shade(
    const scene &the_scene,
    const contact<double> &glb_contact,
    double lambda
) const
{

    const auto &mtl2 = glb_contact.mtl2;
    const auto &mtl3 = glb_contact.mtl3;
    
    shade_info<double> result;
    result.propagation_k = 0.0;
    result.emitted_power = emissivity->value(mtl2, mtl3, lambda)(0);

    return result;
}

std::unique_ptr<ballistae_guile::updatable_material>
guile_ballistae_material(scene *p_scene, SCM config)
{
    auto p = std::make_unique<emitter>();

    p->guile_update(p_scene, config);
    p->emissivity_ind = 1;
    
    return std::move(p);
}
