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
    mtlmap<1> *emissivity;

public:

    emitter();
    virtual ~emitter();

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

emitter::emitter()
{
}

emitter::~emitter()
{
}

void emitter::guile_update(scene *p_scene, SCM config)
{
    using namespace ballistae_guile;

    SCM sym_emissivity = scm_from_utf8_symbol("emissivity");
    SCM lu_emissivity = scm_assq_ref(config, sym_emissivity);

    if(scm_is_true(lu_emissivity))
        this->emissivity = p_scene->mtlmaps_1[scm_to_size_t(lu_emissivity)].get();
}

shade_info<double> emitter::shade(
    const scene &the_scene,
    const contact<double> &glb_contact,
    double lambda_src,
    double lambda_lim,
    double lambda_cur,
    size_t sample_index,
    std::ranlux24 &thread_rng
) const
{

    const auto &mtl2 = glb_contact.mtl2;
    const auto &mtl3 = glb_contact.mtl3;
    
    shade_info<double> result;
    result.propagation_k = 0.0;
    result.emitted_power = emissivity->value(mtl2, mtl3, lambda_cur)(0);

    return result;
}

std::unique_ptr<ballistae_guile::updatable_material>
guile_ballistae_material(scene *p_scene, SCM config)
{
    auto p = std::make_unique<emitter>();

    p->guile_update(p_scene, config);
    p->emissivity = p_scene->mtlmaps_1[1].get();
    
    return std::move(p);
}
