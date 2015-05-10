#include <libballistae/material.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

#include <frustum-0/indicial/fixed.hh>
#include <libguile_frustum0/libguile_frustum0.hh>

#include <libballistae/dense_signal.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

using namespace frustum;
using namespace ballistae;

class directional_emitter : public material
{
public:
    double cutoff;

    double lo_level;
    double hi_level;

    dense_signal<double> spectrum;
    fixvec<double, 3> dir;
public:

    directional_emitter();
    virtual ~directional_emitter();

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

    double cosine = -iprod(dir, glb_contact.r.slope);

    double level = (cosine < cutoff) ? lo_level : hi_level;

    double avg_spectrum = integrate(spectrum, lambda_src, lambda_lim)
        / (lambda_lim - lambda_src);

    shade_info<double> result;
    result.propagation_k = 0.0;
    result.emitted_power = level * avg_spectrum;

    return result;
}

material* guile_ballistae_material(SCM config)
{
    using namespace ballistae_guile;
    using namespace guile_frustum;

    directional_emitter *p = new directional_emitter();

    p->cutoff = 0.0;
    p->spectrum = cie_d65<double>();
    p->dir = {0, 0, -1};

    p->lo_level = 0.0;
    p->hi_level = 1.0;

    guile_ballistae_update_material(p, config);

    return p;
}

material* guile_ballistae_update_material(material *p_matr, SCM config)
{
    using namespace ballistae_guile;
    using namespace guile_frustum;

    directional_emitter *p = dynamic_cast<directional_emitter*>(p_matr);

    SCM sym_spectrum = scm_from_utf8_symbol("spectrum");
    SCM lu_spectrum = scm_assq_ref(config, sym_spectrum);

    SCM sym_dir = scm_from_utf8_symbol("dir");
    SCM lu_dir = scm_assq_ref(config, sym_dir);

    SCM sym_cutoff = scm_from_utf8_symbol("cutoff");
    SCM lu_cutoff = scm_assq_ref(config, sym_cutoff);

    SCM sym_lo_level = scm_from_utf8_symbol("lo-level");
    SCM lu_lo_level = scm_assq_ref(config, sym_lo_level);

    SCM sym_hi_level = scm_from_utf8_symbol("hi-level");
    SCM lu_hi_level = scm_assq_ref(config, sym_hi_level);

    if(scm_is_true(lu_spectrum))
        p->spectrum = signal_from_scm(lu_spectrum);

    if(scm_is_true(lu_dir))
        p->dir = normalise(dvec3_from_scm(lu_dir));

    if(scm_is_true(lu_cutoff))
        p->cutoff = scm_to_double(lu_cutoff);

    if(scm_is_true(lu_lo_level))
        p->lo_level = scm_to_double(lu_lo_level);

    if(scm_is_true(lu_hi_level))
        p->hi_level = scm_to_double(lu_hi_level);

    return p;
}
