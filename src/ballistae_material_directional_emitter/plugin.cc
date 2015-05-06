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

    shade_info<double> result;
    result.propagation_k = 0.0;
    result.emitted_power =
        max(-iprod(glb_contact.n, glb_contact.r.slope), 0.0)
        * integrate(spectrum, lambda_src, lambda_lim);

    return result;
}

material* guile_ballistae_material(SCM config)
{
    using namespace ballistae_guile;
    using namespace guile_frustum;

    directional_emitter *p = new directional_emitter();

    p->spectrum = cie_d65<double>();
    p->dir = {0, 0, 1};

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

    if(scm_is_true(lu_spectrum))
        p->spectrum = signal_from_scm(lu_spectrum);

    if(scm_is_true(lu_dir))
        p->dir = dvec3_from_scm(lu_dir);

    return p;
}
