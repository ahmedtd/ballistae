#include <libballistae/matr_plugin_interface.hh>
#include <libguile_ballistae/matr_plugin_interface.hh>

#include <cmath>

#include <array>
#include <memory>
#include <random>
#include <vector>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/spectrum.hh>
#include <libballistae/uniform_sphere_dist.hh>

#include <libguile_armadillo/libguile_armadillo.hh>
#include <libguile_armadillo/utility.hh>

namespace bl = ballistae;

class phong_priv : public ballistae::matr_priv
{
public:
    double emission_wavelength_mean;
    double emission_wavelength_stddev;
    double emission_peak_power;

    double propagation_wavelength_mean;
    double propagation_wavelength_stddev;
    double propagation_peak_k;
    
public:
    phong_priv();
    virtual ~phong_priv();

    virtual bl::shade_info<double> shade(
        const ballistae::scene &the_scene,
        const ballistae::dray3 &reflected_ray,
        const ballistae::span<double> &contact_span,
        double lambda_nm,
        std::mt19937 &thread_rng
    ) const;
};

phong_priv::phong_priv()
{
}

phong_priv::~phong_priv()
{
}

/// A Gaussian curve that takes the value 1.0 at [mean].
static double normalized_gaussian_density(
    double mean,
    double stddev,
    double x
)
{
    using std::exp;
    using std::pow;

    return exp(-0.5 * pow((x - mean) / stddev, 2));
}

bl::shade_info<double> phong_priv::shade(
    const ballistae::scene &the_scene,
    const ballistae::dray3 &reflected_ray,
    const ballistae::span<double> &contact_span,
    double lambda_nm,
    std::mt19937 &thread_rng
) const
{
    bl::shade_info<double> result;

    const double     &t = contact_span.lo;
    const arma::vec3 &n = contact_span.lo_normal;

    arma::vec3 point = ballistae::eval_ray(reflected_ray, t);

    bl::uniform_hemisphere_dist<double, 3> propagation_dist(1.0, n);
    
    result.incident_ray.slope = propagation_dist(thread_rng);
    result.incident_ray.point = point + 1e-3 * n;
    
    result.propagation_k = arma::dot(result.incident_ray.slope, n)
        * propagation_peak_k
        * normalized_gaussian_density(
            propagation_wavelength_mean,
            propagation_wavelength_stddev,
            lambda_nm
        );

    result.emitted_power = emission_peak_power
        * normalized_gaussian_density(
            emission_wavelength_mean,
            emission_wavelength_stddev,
            lambda_nm
        );

    return result;
}

std::shared_ptr<ballistae::matr_priv> ballistae_matr_create_from_alist(
    SCM config_alist
)
{
    SCM sym_ewm = scm_from_utf8_symbol("emission-wavelength-mean");
    SCM sym_ews = scm_from_utf8_symbol("emission-wavelength-stddev");
    SCM sym_epp = scm_from_utf8_symbol("emission-peak-power");

    SCM sym_pwm = scm_from_utf8_symbol("prop-wavelength-mean");
    SCM sym_pws = scm_from_utf8_symbol("prop-wavelength-stddev");
    SCM sym_ppk = scm_from_utf8_symbol("prop-peak-k");

    // Guile errors will cause memory leaks.

    // Initialize result with default values.
    auto result = std::make_shared<phong_priv>();
    result->emission_wavelength_mean      = 1.0;
    result->emission_wavelength_stddev    = 1.0;
    result->emission_peak_power           = 1.0;
    result->propagation_wavelength_mean   = 1.0;
    result->propagation_wavelength_stddev = 1.0;
    result->propagation_peak_k            = 1.0;
   
    SCM ewm_lu = scm_assq_ref(config_alist, sym_ewm);
    SCM ews_lu = scm_assq_ref(config_alist, sym_ews);
    SCM epp_lu = scm_assq_ref(config_alist, sym_epp);

    SCM pwm_lu = scm_assq_ref(config_alist, sym_pwm);
    SCM pws_lu = scm_assq_ref(config_alist, sym_pws);
    SCM ppk_lu = scm_assq_ref(config_alist, sym_ppk);
   
    if(scm_is_true(ewm_lu))
        result->emission_wavelength_mean = scm_to_double(ewm_lu);

    if(scm_is_true(ews_lu))
        result->emission_wavelength_stddev = scm_to_double(ews_lu);

    if(scm_is_true(epp_lu))
        result->emission_peak_power = scm_to_double(epp_lu);

    if(scm_is_true(pwm_lu))
        result->propagation_wavelength_mean = scm_to_double(pwm_lu);

    if(scm_is_true(pws_lu))
        result->propagation_wavelength_stddev = scm_to_double(pws_lu);

    if(scm_is_true(ppk_lu))
        result->propagation_peak_k = scm_to_double(ppk_lu);

    return result;
}
