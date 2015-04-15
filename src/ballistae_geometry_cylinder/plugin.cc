#include <libballistae/geometry.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>

#include <cmath>

#include <random>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/contact.hh>
#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

#include <libguile_armadillo/libguile_armadillo.hh>

namespace bl = ballistae;

class cylinder_priv : public ballistae::geometry
{
public:
    arma::vec3 center;
    arma::vec3 axis;
    double radius_squared;

public:
    cylinder_priv(
        const arma::vec3 &center_in,
        const arma::vec3 &axis_in,
        const double &radius_in
    );

    virtual ~cylinder_priv();

    virtual bl::contact<double> ray_into(
        const bl::scene &the_scene,
        const bl::ray_segment<double,3> &query,
        std::ranlux24 &thread_rng
    ) const;

    virtual bl::contact<double> ray_exit(
        const bl::scene &the_scene,
        const bl::ray_segment<double,3> &query,
        std::ranlux24 &thread_rng
    ) const;
};

cylinder_priv::cylinder_priv(
    const arma::vec3 &center_in,
    const arma::vec3 &axis_in,
    const double &radius_in
)
    : center(center_in),
      axis(arma::normalise(axis_in)),
      radius_squared(radius_in * radius_in)
{
}

cylinder_priv::~cylinder_priv()
{
}

bl::contact<double> cylinder_priv::ray_into(
    const bl::scene &the_scene,
    const bl::ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    using std::atan2;
    using std::sqrt;

    arma::vec3 foil_a = bl::reject<double, 3>(axis, query.the_ray.slope);
    arma::vec3 foil_b = bl::reject<double, 3>(axis, query.the_ray.point - center);

    double a = arma::dot(foil_a, foil_a);
    double b = 2.0 * arma::dot(foil_a, foil_b);
    double c = arma::dot(foil_b, foil_b) - radius_squared;

    double t_min = (-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a);

    if(!std::isnan(t_min))
    {
        // Ray hits cylinder.

        if(contains(query.the_segment, t_min))
        {
            bl::contact<double> result;

            result.t = t_min;
            result.r = query.the_ray;
            result.p = bl::eval_ray(query.the_ray, t_min);
            result.n = bl::reject<double, 3>(axis, result.p - center);
            result.uv = {result.p(0), std::atan2(result.p(1), result.p(2))};
            result.uvw = result.p;

            return result;
        }
        else
        {
            return bl::contact<double>::nan();
        }
    }
    else
    {
        // Ray misses cylinder.  It could miss outside or inside.

        if(c > double(0))
        {
            return ballistae::contact<double>::nan();
        }
        else
        {
            if(contains(query.the_segment, -std::numeric_limits<double>::infinity()))
            {
                bl::contact<double> result;
                result.t = -std::numeric_limits<double>::infinity();
                result.r = query.the_ray;
                return result;
            }
            else
                return bl::contact<double>::nan();
        }
    }
}

bl::contact<double> cylinder_priv::ray_exit(
    const bl::scene &the_scene,
    const bl::ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    using std::atan2;
    using std::sqrt;

    arma::vec3 foil_a = bl::reject<double, 3>(axis, query.the_ray.slope);
    arma::vec3 foil_b = bl::reject<double, 3>(axis, query.the_ray.point - center);

    double a = arma::dot(foil_a, foil_a);
    double b = 2.0 * arma::dot(foil_a, foil_b);
    double c = arma::dot(foil_b, foil_b) - radius_squared;

    double t_max = (-b + sqrt(b * b - 4.0 * a * c)) / (2.0 * a);

    if(!std::isnan(t_max))
    {
        // The ray hits the cylinder.

        if(contains(query.the_segment, t_max))
        {
            bl::contact<double> result;

            result.t = t_max;
            result.r = query.the_ray;
            result.p = bl::eval_ray(query.the_ray, t_max);
            result.n = bl::reject<double, 3>(axis, result.p - center);
            result.uv = {result.p(0), std::atan2(result.p(1), result.p(2))};
            result.uvw = result.p;

            return result;
        }
        else
        {
            return bl::contact<double>::nan();
        }
    }
    else
    {
        // Ray misses cylinder.  It could miss inside or outside.

        if(contains(query.the_segment, std::numeric_limits<double>::infinity()))
        {
            bl::contact<double> result;
            result.t = std::numeric_limits<double>::infinity();
            result.r = query.the_ray;
            return result;
        }
        else
            return bl::contact<double>::nan();
    }
}

ballistae::geometry* guile_ballistae_geometry(
    SCM config_alist
)
{
    SCM sym_center = scm_from_utf8_symbol("center");
    SCM sym_axis   = scm_from_utf8_symbol("axis");
    SCM sym_radius = scm_from_utf8_symbol("radius");

    SCM cur_tail = config_alist;
    while(cur_tail != SCM_EOL)
    {
        SCM cur_key = scm_caar(cur_tail);
        SCM cur_val = scm_cdar(cur_tail);
        cur_tail = scm_cdr(cur_tail);

        if(scm_is_true(scm_eq_p(sym_center, cur_key))
           || scm_is_true(scm_eq_p(sym_axis, cur_key)))
        {
            SCM_ASSERT_TYPE(
                scm_is_true(
                    arma_guile::generic_col_dim_p<double>(
                        cur_val,
                        scm_from_int(3)
                    )
                ),
                cur_val,
                SCM_ARGn,
                nullptr,
                "arma/b64col[3]"
            );
        }
        else if(scm_is_true(scm_eq_p(sym_radius, cur_key)))
        {
            SCM_ASSERT_TYPE(
                scm_is_true(scm_real_p(cur_val)),
                cur_val,
                SCM_ARGn,
                nullptr,
                "real"
            );
        }
        else
        {
            scm_wrong_type_arg_msg(nullptr, SCM_ARGn, cur_key, "Unknown key.");
        }
    }

    // No guile errors below this point.

    auto cyl_p = new cylinder_priv(
        arma::vec3({0, 0, 0}),
        arma::vec3({0, 0, 1}),
        1.0
    );

    SCM center_lookup = scm_assq_ref(config_alist, sym_center);
    SCM axis_lookup   = scm_assq_ref(config_alist, sym_axis);
    SCM radius_lookup = scm_assq_ref(config_alist, sym_radius);

    if(scm_is_true(center_lookup))
    {
        cyl_p->center = arma_guile::extract_col<double>(center_lookup);
    }

    if(scm_is_true(axis_lookup))
    {
        cyl_p->axis = arma::normalise(
            arma_guile::extract_col<double>(center_lookup)
        );
    }

    if(scm_is_true(radius_lookup))
    {
        cyl_p->radius_squared = scm_to_double(radius_lookup);
    }

    return cyl_p;
}
