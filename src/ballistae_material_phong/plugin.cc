#include <libballistae/material.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

#include <libballistae/dense_signal.hh>

#include <libguile_ballistae/dense_signal.hh>

namespace bl = ballistae;

class phong_priv : public ballistae::material
{
public:
    bl::dense_signal<double> ambient;
    bl::dense_signal<double> diffuse;
    bl::dense_signal<double> specular;
    bl::dense_signal<double> reflect;

public:

    phong_priv();
    virtual ~phong_priv();

    virtual bl::shade_info<double> shade(
        const bl::scene &the_scene,
        const bl::contact<double> &glb_contact,
        double lambda_nm,
        size_t sample_index,
        std::ranlux24 &thread_rng
    ) const;
};

phong_priv::phong_priv()
{
}

phong_priv::~phong_priv()
{
}

bl::shade_info<double> phong_priv::shade(
    const bl::scene &the_scene,
    const bl::contact<double> &glb_contact,
    double lambda_nm,
    size_t sample_index,
    std::ranlux24 &thread_rng
) const
{
    using arma::dot;
    
    bl::shade_info<double> result;
    result.propagation_k = 0.0;
    result.emitted_power = 0.0;

    result.emitted_power += ambient(lambda_nm);

    for(const auto &illum_p : the_scene.illuminators)
    {
        auto info = illum_p->power_at_point(
            the_scene,
            glb_contact.p,
            lambda_nm,
            thread_rng
        );

        double cosine = -dot(glb_contact.n, info.arrival);
        if(cosine > 0.0)
            result.emitted_power += cosine * info.power * diffuse(lambda_nm);
    }

    return result;
}

ballistae::material* guile_ballistae_material(SCM config_alist)
{
    SCM sym_ambient = scm_from_utf8_symbol("ambient");
    SCM sym_diffuse = scm_from_utf8_symbol("diffuse");
    SCM sym_specular = scm_from_utf8_symbol("specular");

    SCM lu_ambient = scm_assq_ref(config_alist, sym_ambient);
    SCM lu_diffuse = scm_assq_ref(config_alist, sym_diffuse);
    SCM lu_specular = scm_assq_ref(config_alist, sym_specular);

    phong_priv *result = new phong_priv();

    if(scm_is_true(lu_ambient))
        result->ambient = ballistae_guile::dense_signal::from_scm(lu_ambient);
    else
        result->ambient = bl::vis_spectrum_signal<double>(); // All zeroes
            
    if(scm_is_true(lu_diffuse))
        result->diffuse = ballistae_guile::dense_signal::from_scm(lu_diffuse);
    else
        result->diffuse = bl::vis_spectrum_signal<double>(); // All zeroes

    if(scm_is_true(lu_specular))
        result->specular = ballistae_guile::dense_signal::from_scm(lu_specular);
    else
        result->specular = bl::vis_spectrum_signal<double>(); // All zeroes

    return result;
}
