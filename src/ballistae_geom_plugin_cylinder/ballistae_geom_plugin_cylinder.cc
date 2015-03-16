#include <libballistae/geom_plugin_interface.hh>
#include <libguile_ballistae/geom_plugin_interface.hh>

#include <cmath>

#include <memory>
#include <random>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

#include <libguile_armadillo/libguile_armadillo.hh>

namespace bl = ballistae;

class cylinder_priv : public ballistae::geom_priv
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

    virtual bl::span<double> ray_intersect(
        const bl::scene &the_scene,
        const bl::dray3 &query,
        const bl::span<double> &must_overlap,
        std::mt19937 &thread_rng
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

bl::span<double> cylinder_priv::ray_intersect(
    const bl::scene &the_scene,
    const bl::dray3 &query,
    const bl::span<double> &must_overlap,
    std::mt19937 &thread_rng
) const
{
    using std::sqrt;

    arma::vec3 foil_a = bl::reject<double, 3>(axis, query.slope);
    arma::vec3 foil_b = bl::reject<double, 3>(axis, query.point - center);

    double a = arma::dot(foil_a, foil_a);
    double b = 2.0 * arma::dot(foil_a, foil_b);
    double c = arma::dot(foil_b, foil_b) - radius_squared;

    double t_min = (-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a);
    double t_max = (-b + sqrt(b * b - 4.0 * a * c)) / (2.0 * a);

    auto covered = bl::span<double>::undecorated(t_min, t_max);

    if(!std::isnan(t_min))
    {
        // The ray hits the cylinder.
        if(overlaps(must_overlap, covered))
        {
            arma::vec3 p_min = ballistae::eval_ray(query, t_min);
            arma::vec3 p_max = ballistae::eval_ray(query, t_max);

            covered.lo_normal = bl::reject<double, 3>(axis, p_min - center);
            covered.hi_normal = bl::reject<double, 3>(axis, p_max - center);

            return covered;
        }
        else
        {
            return ballistae::span<double>::nan();
        }
    }
    else
    {
        // The ray is parallel to the cylinder axis.

        if(c > double(0))
        {
            return ballistae::span<double>::nan();
        }
        else
        {
            double inf = std::numeric_limits<double>::infinity();
            auto test = ballistae::span<double>::undecorated(-inf, inf);

            if(overlaps(must_overlap, test))
                return test;
            else
                return bl::span<double>::nan();
        }
    }
}

std::shared_ptr<ballistae::geom_priv> ballistae_geom_create_from_alist(
    SCM config_alist
)
{
    constexpr const char* const subr = "ballistae_geom_create_from_alist(cylinder)";

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
                subr,
                "arma/b64col[3]"
            );
        }
        else if(scm_is_true(scm_eq_p(sym_radius, cur_key)))
        {
            SCM_ASSERT_TYPE(
                scm_is_true(scm_real_p(cur_val)),
                cur_val,
                SCM_ARGn,
                subr,
                "real"
            );
        }
        else
        {
            scm_wrong_type_arg_msg(subr, SCM_ARGn, cur_key, "Unknown key.");
        }
    }

    // No guile errors below this point.

    auto cyl_p = std::make_shared<cylinder_priv>(
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
