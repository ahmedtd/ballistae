#include <libballistae/matr_plugin_interface.hh>
#include <libguile_ballistae/matr_plugin_interface.hh>

#include <cmath>

#include <memory>
#include <vector>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/ray.hh>
#include <libballistae/span.hh>
#include <libballistae/spectrum.hh>

#include <libguile_armadillo/libguile_armadillo.hh>
#include <libguile_armadillo/utility.hh>

class phong_priv : public ballistae::matr_priv
{
public:
    double k_a;
    double k_d;
    double k_s;

    double d_max;
    double d_min;

    double s_max;
    double s_min;

    double alpha;

    std::vector<arma::vec3> lights;

    ballistae::color_d_rgb color_a;
    ballistae::color_d_rgb color_d;
    ballistae::color_d_rgb color_s;
    
public:
    phong_priv();
    virtual ~phong_priv();

    virtual ballistae::color_d_rgb shade(
        const ballistae::dray3 &eye_ray,
        const ballistae::span<double> &span,
        const arma::vec3 *span_normals
    ) const;
};

phong_priv::phong_priv()
{
}

phong_priv::~phong_priv()
{
}

ballistae::color_d_rgb phong_priv::shade(
    const ballistae::dray3 &eye_ray,
    const ballistae::span<double> &span,
    const arma::vec3 *span_normals
) const
{
    ballistae::color_d_rgb result = {{0.0, 0.0, 0.0}};

    if(std::isnan(span.lo))
        return result;

    const double &t = span.lo;
    const arma::vec3 &n = span_normals[0];
    
    result = result +  k_a * color_a;

    for(const auto &light : lights)
    {
        arma::vec3 light_v = light - ballistae::eval_ray(eye_ray, t);
        arma::vec3 light_n = arma::normalise(light_v);
        double diffuse_contr = arma::dot(n, light_n);
        diffuse_contr = (diffuse_contr - d_min) / (d_max - d_min);
        
        arma::vec3 refl
                = 2.0 * arma::dot(light_n, n) * n - light_n;

        double specular_contr = arma::dot(refl, -eye_ray.slope);
        specular_contr = (specular_contr - s_min) / (s_max - s_min);
        
        if(diffuse_contr > 0.0 && diffuse_contr < 1.0)
        {
            result = result + diffuse_contr * k_d * color_d;

            if(specular_contr > s_min && specular_contr < s_max)
            {
                specular_contr = std::pow(specular_contr, alpha);

                result = result + specular_contr * k_s * color_s;
            }
        }
    }
    
    return result;
}

std::shared_ptr<ballistae::matr_priv> ballistae_matr_create_from_alist(
    SCM config_alist
)
{
    constexpr const char* const subr
        = "ballistae_matr_create_from_alist(phong)";

    SCM sym_k_a = scm_from_utf8_symbol("k_a");
    SCM sym_k_d = scm_from_utf8_symbol("k_d");
    SCM sym_k_s = scm_from_utf8_symbol("k_s");

    SCM sym_d_max = scm_from_utf8_symbol("d_max");
    SCM sym_d_min = scm_from_utf8_symbol("d_min");
    SCM sym_s_max = scm_from_utf8_symbol("s_max");
    SCM sym_s_min = scm_from_utf8_symbol("s_min");
    
    SCM sym_alpha = scm_from_utf8_symbol("alpha");

    SCM sym_lights = scm_from_utf8_symbol("lights");

    SCM sym_color_a = scm_from_utf8_symbol("color_a");
    SCM sym_color_d = scm_from_utf8_symbol("color_d");
    SCM sym_color_s = scm_from_utf8_symbol("color_s");
    
    SCM cur_head = config_alist;
    while(! scm_is_null(cur_head))
    {
        SCM cur_key = scm_caar(cur_head);
        SCM cur_val = scm_cdar(cur_head);
        cur_head = scm_cdr(cur_head);

        if(scm_is_true(scm_eq_p(sym_k_a, cur_key))
           || scm_is_true(scm_eq_p(sym_k_d, cur_key))
           || scm_is_true(scm_eq_p(sym_k_s, cur_key))
           || scm_is_true(scm_eq_p(sym_d_max, cur_key))
           || scm_is_true(scm_eq_p(sym_d_min, cur_key))
           || scm_is_true(scm_eq_p(sym_s_max, cur_key))
           || scm_is_true(scm_eq_p(sym_s_min, cur_key))
           || scm_is_true(scm_eq_p(sym_alpha, cur_key)))
        {
            SCM_ASSERT_TYPE(
                scm_is_true(scm_real_p(cur_val)),
                cur_val,
                SCM_ARGn,
                subr,
                "real"
            );
        }
        else if(scm_is_true(scm_eq_p(sym_lights, cur_key)))
        {
            SCM_ASSERT_TYPE(
                scm_is_true(scm_list_p(cur_val)),
                cur_val,
                SCM_ARGn,
                subr,
                "list of lights"
            );

            SCM cur_tail = cur_val;
            while(! scm_is_null(cur_tail))
            {
                SCM cur_elem = scm_car(cur_tail);
                cur_tail = scm_cdr(cur_tail);

                SCM_ASSERT_TYPE(
                    scm_is_true(
                        arma_guile::generic_col_dim_p<double>(
                            cur_elem,
                            scm_from_size_t(3)
                        )
                    ),
                    cur_elem,
                    SCM_ARGn,
                    subr,
                    "arma/f64col[3]"
                );
            }
        }
        else if(scm_is_true(scm_eq_p(sym_color_a, cur_key))
                || scm_is_true(scm_eq_p(sym_color_d, cur_key))
                || scm_is_true(scm_eq_p(sym_color_s, cur_key)))
        {
            SCM_ASSERT_TYPE(
                scm_is_true(scm_list_p(cur_val))
                && scm_to_size_t(scm_length(cur_val)) == 3,
                cur_val,
                SCM_ARGn,
                subr,
                "list of 3 reals"
            );

            SCM cur_tail = cur_val;
            while(! scm_is_null(cur_tail))
            {
                SCM cur_elem = scm_car(cur_tail);
                cur_tail = scm_cdr(cur_tail);

                SCM_ASSERT_TYPE(
                    scm_is_true(scm_real_p(cur_elem)),
                    cur_elem,
                    SCM_ARGn,
                    subr,
                    "real"
                );
            }
        }
        else
        {
            scm_wrong_type_arg_msg(subr, SCM_ARGn, cur_key, "Unknown key.");
        }
    }

    // No guile errors below this point.

    // Initialize result with default values.
    auto result = std::make_shared<phong_priv>();
    result->k_a = 1.0;
    result->k_d = 1.0;
    result->k_s = 1.0;
    result->d_max = 1.0;
    result->d_min = 0.0;
    result->s_max = 1.0;
    result->s_min = 0.0;
    result->alpha = 0.2;
    result->color_a = {1.0, 1.0, 1.0};
    result->color_d = {1.0, 1.0, 1.0};
    result->color_s = {1.0, 1.0, 1.0};

    SCM k_a_lookup = scm_assq_ref(config_alist, sym_k_a);
    SCM k_d_lookup = scm_assq_ref(config_alist, sym_k_d);
    SCM k_s_lookup = scm_assq_ref(config_alist, sym_k_s);

    SCM d_max_lookup = scm_assq_ref(config_alist, sym_d_max);
    SCM d_min_lookup = scm_assq_ref(config_alist, sym_d_min);
    SCM s_max_lookup = scm_assq_ref(config_alist, sym_s_max);
    SCM s_min_lookup = scm_assq_ref(config_alist, sym_s_min);

    SCM alpha_lookup = scm_assq_ref(config_alist, sym_alpha);
    SCM lights_lookup = scm_assq_ref(config_alist, sym_lights);
    SCM color_a_lookup = scm_assq_ref(config_alist, sym_color_a);
    SCM color_d_lookup = scm_assq_ref(config_alist, sym_color_d);
    SCM color_s_lookup = scm_assq_ref(config_alist, sym_color_s);
    
    if(scm_is_true(k_a_lookup))
    {
        result->k_a = scm_to_double(k_a_lookup);
    }

    if(scm_is_true(k_d_lookup))
    {
        result->k_d = scm_to_double(k_d_lookup);
    }

    if(scm_is_true(k_s_lookup))
    {
        result->k_s = scm_to_double(k_s_lookup);
    }

    if(scm_is_true(d_max_lookup))
    {
        result->d_max = scm_to_double(d_max_lookup);
    }

    if(scm_is_true(d_min_lookup))
    {
        result->d_min = scm_to_double(d_min_lookup);
    }

    if(scm_is_true(s_max_lookup))
    {
        result->s_max = scm_to_double(s_max_lookup);
    }

    if(scm_is_true(s_min_lookup))
    {
        result->s_min = scm_to_double(s_min_lookup);
    }
    
    if(scm_is_true(alpha_lookup))
    {
        result->alpha = scm_to_double(alpha_lookup);
    }

    if(scm_is_true(lights_lookup))
    {
        SCM cur_tail = lights_lookup;
        while(! scm_is_null(cur_tail))
        {
            SCM cur_light = scm_car(cur_tail);
            cur_tail = scm_cdr(cur_tail);

            result->lights.push_back(
                *arma_guile::smob_get_data<arma::Col<double>*>(cur_light)
            );
        }
    }

    if(scm_is_true(color_a_lookup))
    {
        std::size_t i = 0;
        SCM cur_tail = color_a_lookup;
        while(! scm_is_null(cur_tail))
        {
            result->color_a.channels[i] = scm_to_double(scm_car(cur_tail));
            cur_tail = scm_cdr(cur_tail);
            ++i;
        }
    }

    if(scm_is_true(color_d_lookup))
    {
        std::size_t i = 0;
        SCM cur_tail = color_d_lookup;
        while(! scm_is_null(cur_tail))
        {
            result->color_d.channels[i] = scm_to_double(scm_car(cur_tail));
            cur_tail = scm_cdr(cur_tail);
            ++i;
        }
    }
    
    if(scm_is_true(color_s_lookup))
    {
        std::size_t i = 0;
        SCM cur_tail = color_s_lookup;
        while(! scm_is_null(cur_tail))
        {
            result->color_s.channels[i] = scm_to_double(scm_car(cur_tail));
            cur_tail = scm_cdr(cur_tail);
            ++i;
        }
    }

    return result;
}

