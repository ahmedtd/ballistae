#include <libballistae/material.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/dense_signal.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

using namespace frustum;
using namespace ballistae;

class omnidirectional_emitter : public material
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
};

omnidirectional_emitter::omnidirectional_emitter()
{
}

omnidirectional_emitter::~omnidirectional_emitter()
{
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
    result.emitted_power = avg_spectrum;

    return result;
}

material* guile_ballistae_material(SCM config)
{
    omnidirectional_emitter *p = new omnidirectional_emitter();

    p->spectrum = cie_d65<double>();
    p->level = 1.0;

    guile_ballistae_update_material(p, config);

    return p;
}

material* guile_ballistae_update_material(material *p_matr, SCM config)
{
    using namespace ballistae_guile;

    omnidirectional_emitter *p = dynamic_cast<omnidirectional_emitter*>(p_matr);

    SCM sym_spectrum = scm_from_utf8_symbol("spectrum");
    SCM lu_spectrum = scm_assq_ref(config, sym_spectrum);

    SCM sym_level = scm_from_utf8_symbol("level");
    SCM lu_level = scm_assq_ref(config, sym_level);

    if(scm_is_true(lu_spectrum))
        p->spectrum = signal_from_scm(lu_spectrum);

    if(scm_is_true(lu_level))
        p->level = scm_to_double(lu_level);

    return p;
}
