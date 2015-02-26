#include <libballistae/matr_plugin_interface.hh>
#include <libguile_ballistae/matr_plugin_interface.hh>

#include <cmath>

#include <array>
#include <memory>
#include <vector>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/spectrum.hh>

#include <libguile_armadillo/libguile_armadillo.hh>
#include <libguile_armadillo/utility.hh>

struct spot_light
{
    arma::vec3 pos;
    arma::vec3 dir;
    double     cos_cutoff_angle;
};

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

    std::vector<arma::vec3> point_lights;
    std::vector<arma::vec3> dir_lights;
    std::vector<spot_light> spot_lights;
    
    ballistae::color_d_rgb color_a;
    ballistae::color_d_rgb color_d;
    ballistae::color_d_rgb color_s;
    
public:
    phong_priv();
    virtual ~phong_priv();

    virtual void shade(
        const ballistae::scene &the_scene,
        const ballistae::dray3 *eyes_src,
        const ballistae::dray3 *eyes_lim,
        const ballistae::span<double> *spans_src,
        const arma::vec3 *normals_src,
        ballistae::color_d_rgb *shades_out_src
    ) const;
};

phong_priv::phong_priv()
{
}

phong_priv::~phong_priv()
{
}

void phong_priv::shade(
    const ballistae::scene &the_scene,
    const ballistae::dray3 *eyes_src,
    const ballistae::dray3 *eyes_lim,
    const ballistae::span<double> *spans_src,
    const arma::vec3 *normals_src,
    ballistae::color_d_rgb *shades_out_src
) const
{
    for(; eyes_src != eyes_lim;
        ++eyes_src, ++spans_src, normals_src += 2, ++shades_out_src)
    {
        // If the geometry didn't contribute, just pass on the whole thing.
        if(std::isnan(spans_src->lo))
            continue;

        // Ambient contribution.
        *shades_out_src += k_a * color_a;
        
        const double     &t = spans_src->lo;
        const arma::vec3 &n = normals_src[0];
        
        arma::vec3 point = ballistae::eval_ray(*eyes_src, t);

        for(const auto &light : point_lights)
        {
            arma::vec3 light_vec = light - point;
            double light_distance = arma::norm(light_vec);
            ballistae::dray3 light_ray = {point, light_vec / light_distance};

            ballistae::span<double> shadow_span {
                -ballistae::span<double>::inf(),
                0
            };
            std::array<arma::vec3, 2> shadow_normals;
            std::size_t               shadow_geomind;

            while(shadow_span.lo < 0) {
                ballistae::ray_intersect_batch(
                    &light_ray,
                    &light_ray + 1,
                    {shadow_span.hi, ballistae::span<double>::inf()},
                    the_scene,
                    &shadow_span,
                    shadow_normals.data(),
                    &shadow_geomind
                );
            }

            // Test if the point is in shadow.
            if(shadow_span.lo < light_distance)
                continue;

            double diffuse_contr = arma::dot(n, light_ray.slope);
            diffuse_contr = (diffuse_contr - d_min) / (d_max - d_min);

            if(!(diffuse_contr > 0.0 && diffuse_contr < 1.0))
                continue;

            *shades_out_src += diffuse_contr * k_d * color_d;

            arma::vec3 refl = 2.0 * arma::dot(light_ray.slope, n) * n
                - light_ray.slope;
            double specular_contr = arma::dot(refl, -(eyes_src->slope));
            specular_contr = (specular_contr - s_min) / (s_max - s_min);
            
            if(!(specular_contr > s_min && specular_contr < s_max))
                continue;
            
            specular_contr = std::pow(specular_contr, alpha);
                
            *shades_out_src += specular_contr * k_s * color_s;
        }

        for(const auto &light : dir_lights)
        {
            ballistae::dray3 light_ray = {point, -light};

            ballistae::span<double> shadow_span {
                -ballistae::span<double>::inf(),
                0
            };
            std::array<arma::vec3, 2> shadow_normals;
            std::size_t               shadow_geomind;

            while(shadow_span.lo < 0) {
                ballistae::ray_intersect_batch(
                    &light_ray,
                    &light_ray + 1,
                    {shadow_span.hi, ballistae::span<double>::inf()},
                    the_scene,
                    &shadow_span,
                    shadow_normals.data(),
                    &shadow_geomind
                );
            }

            // Test if the point is in shadow.
            if(shadow_span.lo < std::numeric_limits<double>::infinity())
                continue;

            double diffuse_contr = arma::dot(n, light_ray.slope);
            diffuse_contr = (diffuse_contr - d_min) / (d_max - d_min);

            if(!(diffuse_contr > 0.0 && diffuse_contr < 1.0))
                continue;

            *shades_out_src += diffuse_contr * k_d * color_d;

            arma::vec3 refl = 2.0 * arma::dot(light_ray.slope, n) * n
                - light_ray.slope;
            double specular_contr = arma::dot(refl, -(eyes_src->slope));
            specular_contr = (specular_contr - s_min) / (s_max - s_min);
            
            if(!(specular_contr > s_min && specular_contr < s_max))
                continue;
            
            specular_contr = std::pow(specular_contr, alpha);
                
            *shades_out_src += specular_contr * k_s * color_s;
        }

        for(const auto &light : spot_lights)
        {
            ballistae::dray3 light_ray = {point, light.pos - point};
            double light_distance = arma::norm(light_ray.slope);
            light_ray.slope /= light_distance;

            // Test point is outside of the light's cutoff.
            if(arma::dot(-light_ray.slope, light.dir) < light.cos_cutoff_angle)
                continue;
            
            ballistae::span<double> shadow_span {
                -ballistae::span<double>::inf(),
                0
            };
            std::array<arma::vec3, 2> shadow_normals;
            std::size_t               shadow_geomind;

            while(shadow_span.lo < 0) {
                ballistae::ray_intersect_batch(
                    &light_ray,
                    &light_ray + 1,
                    {shadow_span.hi, ballistae::span<double>::inf()},
                    the_scene,
                    &shadow_span,
                    shadow_normals.data(),
                    &shadow_geomind
                );
            }

            // Test if the point is in shadow.
            if(shadow_span.lo < light_distance)
                continue;

            double diffuse_contr = arma::dot(n, light_ray.slope);
            diffuse_contr = (diffuse_contr - d_min) / (d_max - d_min);

            if(!(diffuse_contr > 0.0 && diffuse_contr < 1.0))
                continue;

            *shades_out_src += diffuse_contr * k_d * color_d;

            arma::vec3 refl = 2.0 * arma::dot(light_ray.slope, n) * n
                - light_ray.slope;
            double specular_contr = arma::dot(refl, -(eyes_src->slope));
            specular_contr = (specular_contr - s_min) / (s_max - s_min);
            
            if(!(specular_contr > s_min && specular_contr < s_max))
                continue;
            
            specular_contr = std::pow(specular_contr, alpha);
                
            *shades_out_src += specular_contr * k_s * color_s;
        }
    }
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

    SCM sym_point_lights = scm_from_utf8_symbol("point-lights");
    SCM sym_dir_lights = scm_from_utf8_symbol("dir-lights");
    SCM sym_spot_lights = scm_from_utf8_symbol("spot-lights");
    
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
        else if(scm_is_true(scm_eq_p(sym_point_lights, cur_key)))
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
        else if(scm_is_true(scm_eq_p(sym_dir_lights, cur_key)))
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
        else if(scm_is_true(scm_eq_p(sym_spot_lights, cur_key)))
        {
            SCM_ASSERT_TYPE(
                scm_is_true(scm_list_p(cur_val)),
                cur_val,
                SCM_ARGn,
                subr,
                "list of lights"
            );

            // SCM cur_tail = cur_val;
            // while(! scm_is_null(cur_tail))
            // {
            //     SCM cur_elem = scm_car(cur_tail);
            //     cur_tail = scm_cdr(cur_tail);

            //     // What form should a spotlight have?
            // }
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

    SCM point_lights_lookup = scm_assq_ref(config_alist, sym_point_lights);
    SCM dir_lights_lookup = scm_assq_ref(config_alist, sym_dir_lights);
    SCM spot_lights_lookup = scm_assq_ref(config_alist, sym_spot_lights);
    
    SCM alpha_lookup = scm_assq_ref(config_alist, sym_alpha);
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

    if(scm_is_true(point_lights_lookup))
    {
        SCM cur_tail = point_lights_lookup;
        while(! scm_is_null(cur_tail))
        {
            SCM cur_light = scm_car(cur_tail);
            cur_tail = scm_cdr(cur_tail);

            result->point_lights.push_back(
                *arma_guile::smob_get_data<arma::Col<double>*>(cur_light)
            );
        }
    }

    if(scm_is_true(dir_lights_lookup))
    {
        SCM cur_tail = dir_lights_lookup;
        while(! scm_is_null(cur_tail))
        {
            SCM cur_light = scm_car(cur_tail);
            cur_tail = scm_cdr(cur_tail);

            result->dir_lights.push_back(
                arma::normalise(
                    *arma_guile::smob_get_data<arma::Col<double>*>(cur_light)
                )
            );
        }
    }

    if(scm_is_true(spot_lights_lookup))
    {
        using std::cos;
        
        SCM cur_tail = spot_lights_lookup;
        while(cur_tail != SCM_EOL)
        {
            SCM cur_light = scm_car(cur_tail);
            cur_tail = scm_cdr(cur_tail);

            result->spot_lights.push_back({
                *arma_guile::smob_get_dcol(scm_assq_ref(cur_light, scm_from_utf8_symbol("pos"))),
                arma::normalise(*arma_guile::smob_get_dcol(scm_assq_ref(cur_light, scm_from_utf8_symbol("dir")))),
                cos(scm_to_double(scm_assq_ref(cur_light, scm_from_utf8_symbol("cutoff-angle"))))
            });
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

