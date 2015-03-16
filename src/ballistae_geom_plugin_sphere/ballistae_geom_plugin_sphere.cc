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

#include <libguile_armadillo/libguile_armadillo.hh>

namespace bl = ballistae;

class sphere_priv : public ballistae::geom_priv
{
public:
    arma::vec3 center;
    double radius_squared;

    sphere_priv(
        const arma::vec3 &center_in,
        const double &radius_squared_in
    );

    virtual ~sphere_priv();

    virtual bl::span<double> ray_intersect(
        const bl::scene &the_scene,
        const bl::dray3 &query,
        const bl::span<double> &must_overlap,
        std::mt19937 &thread_rng
    ) const;
};

sphere_priv::sphere_priv(
    const arma::vec3 &center_in,
    const double &radius_squared_in
)
    : center(center_in),
      radius_squared(radius_squared_in)
{
}

sphere_priv::~sphere_priv()
{
}

bl::span<double> sphere_priv::ray_intersect(
    const bl::scene &the_scene,
    const bl::dray3 &query,
    const bl::span<double> &must_overlap,
    std::mt19937 &thread_rng
) const
{
    using std::sqrt;

    auto offset = query.point - this->center;

    auto b = arma::dot(query.slope, offset);
    auto c = arma::dot(offset, offset) - this->radius_squared;

    // We rely on std::sqrt's mandated NaN behavior.
    auto t_min = -b - sqrt(b * b - c);
    auto t_max = -b + sqrt(b * b - c);

    auto covered = bl::span<double>::undecorated(t_min, t_max);

    if(ballistae::overlaps(must_overlap, covered))
    {
        auto point_min = ballistae::eval_ray(query, t_min);
        auto point_max = ballistae::eval_ray(query, t_max);

        covered.lo_normal = arma::normalise(point_min - this->center);
        covered.hi_normal = arma::normalise(point_max - this->center);

        return covered;
    }
    else
    {
        return bl::span<double>::nan();
    }
}

std::shared_ptr<ballistae::geom_priv> ballistae_geom_create_from_alist(
    SCM config_alist
)
{
    constexpr const char* const subr = "ballistae_geom_create_from_alist(sphere)";

    SCM sym_center = scm_from_utf8_symbol("center");
    SCM sym_radius = scm_from_utf8_symbol("radius");

    SCM cur_head = config_alist;
    while(! scm_is_null(cur_head))
    {
        SCM cur_key = scm_caar(cur_head);
        SCM cur_val = scm_cdar(cur_head);
        cur_head = scm_cdr(cur_head);

        if(scm_is_true(scm_eq_p(sym_center, cur_key)))
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
                "Key 'center requires arma/b64col[3] argument."
            );
        }
        else if(scm_is_true(scm_eq_p(sym_radius, cur_key)))
        {
            SCM_ASSERT_TYPE(
                scm_is_true(scm_real_p(cur_val)),
                cur_val,
                SCM_ARGn,
                subr,
                "Key 'radius requires real argument."
            );
        }
        else
        {
            scm_wrong_type_arg_msg(subr, SCM_ARGn, cur_key, "Unknown key.");
        }
    }

    // No guile errors below this point.

    arma::vec3 center(arma::fill::zeros);
    double radius_squared = 1.0;

    SCM center_lookup = scm_assq_ref(config_alist, sym_center);
    SCM radius_lookup = scm_assq_ref(config_alist, sym_radius);

    if(scm_is_true(center_lookup))
    {
        auto col_p = arma_guile::smob_get_data<arma::Col<double>*>(
            center_lookup
        );
        center = *col_p;
    }

    if(scm_is_true(radius_lookup))
    {
        auto radius = scm_to_double(radius_lookup);
        radius_squared = radius * radius;
    }

    return std::make_shared<sphere_priv>(center, radius_squared);
}
