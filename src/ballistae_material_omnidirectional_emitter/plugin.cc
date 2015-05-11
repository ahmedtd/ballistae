#include <libballistae/material.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/dense_signal.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

using namespace frustum;
using namespace ballistae;

class omnidirectional_emitter : public ballistae_guile::updatable_material
{
public:
    double level;
    dense_signal<double> spectrum;

public:

    omnidirectional_emitter();
    virtual ~omnidirectional_emitter();

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

omnidirectional_emitter::omnidirectional_emitter()
{
}

omnidirectional_emitter::~omnidirectional_emitter()
{
}

void omnidirectional_emitter::guile_update(scene *p_scene, SCM config)
{
    using namespace ballistae_guile;

    SCM sym_spectrum = scm_from_utf8_symbol("spectrum");
    SCM lu_spectrum = scm_assq_ref(config, sym_spectrum);

    SCM sym_level = scm_from_utf8_symbol("level");
    SCM lu_level = scm_assq_ref(config, sym_level);

    if(scm_is_true(lu_spectrum))
        this->spectrum = signal_from_scm(lu_spectrum);

    if(scm_is_true(lu_level))
        this->level = scm_to_double(lu_level);
}

shade_info<double> omnidirectional_emitter::shade(
    const scene &the_scene,
    const contact<double> &glb_contact,
    double lambda_src,
    double lambda_lim,
    double lambda_cur,
    size_t sample_index,
    std::ranlux24 &thread_rng
) const
{
    double avg_spectrum = integrate(spectrum, lambda_src, lambda_lim)
        / (lambda_lim - lambda_src);

    shade_info<double> result;
    result.propagation_k = 0.0;
    result.emitted_power = level * avg_spectrum;

    return result;
}

ballistae_guile::updatable_material*
guile_ballistae_material(scene *p_scene, SCM config)
{
    omnidirectional_emitter *p = new omnidirectional_emitter();

    p->spectrum = cie_d65<double>();
    p->level = 1.0;

    p->guile_update(p_scene, config);

    return p;
}
