#include <libballistae/material.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/dense_signal.hh>
#include <libballistae/material_map.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

using namespace frustum;
using namespace ballistae;

/// An emitter that queries an emissivity material map based on direction of
/// arrival.  In effect, it acts as a window into an environment map.
class directional_emitter : public ballistae_guile::updatable_material
{
public:
    mtlmap<1> *emissivity;

    directional_emitter();
    virtual ~directional_emitter();

    virtual void guile_update(scene *p_scene, SCM config);

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

directional_emitter::directional_emitter()
{
}

directional_emitter::~directional_emitter()
{
}

void directional_emitter::guile_update(scene *p_scene, SCM config)
{
    using namespace ballistae_guile;

    SCM sym_emissivity = scm_from_utf8_symbol("emissivity");
    SCM lu_emissivity = scm_assq_ref(config, sym_emissivity);

    if(scm_is_true(lu_emissivity))
        this->emissivity = p_scene->mtlmaps_1[scm_to_size_t(lu_emissivity)].get();
}

shade_info<double> directional_emitter::shade(
    const scene &the_scene,
    const contact<double> &glb_contact,
    double lambda_src,
    double lambda_lim,
    double lambda_cur,
    size_t sample_index,
    std::ranlux24 &thread_rng
) const
{
    using std::max;

    const auto &r = glb_contact.r.slope;
    fixvec<double, 3> mtl3 = r;
    fixvec<double, 2> mtl2 = {atan2(r(0), r(1)), acos(r(2))};

    shade_info<double> result;
    result.propagation_k = 0.0;
    result.emitted_power = emissivity->value(mtl2, mtl3, lambda_cur)(0);

    return result;
}

std::unique_ptr<ballistae_guile::updatable_material>
guile_ballistae_material(scene *p_scene, SCM config)
{
    using namespace ballistae_guile;

    auto p = std::make_unique<directional_emitter>();
    p->emissivity = p_scene->mtlmaps_1[1].get();

    p->guile_update(p_scene, config);

    return std::move(p);
}
