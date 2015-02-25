#include <libballistae/geom_plugin_interface.hh>
#include <libguile_ballistae/geom_plugin_interface.hh>

#include <cmath>

#include <memory>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/ray.hh>
#include <libballistae/span.hh>

#include <libguile_armadillo/libguile_armadillo.hh>

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

    virtual void ray_intersect(
        const ballistae::dray3 *query_src,
        const ballistae::dray3 *query_lim,
        const ballistae::span<double> &must_overlap,
        const std::size_t index,
        ballistae::span<double> *out_spans_src,
        arma::vec3 *out_normals_src
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

void sphere_priv::ray_intersect(
    const ballistae::dray3 *query_src,
    const ballistae::dray3 *query_lim,
    const ballistae::span<double> &must_overlap,
    const std::size_t index,
    ballistae::span<double> *out_spans_src,
    arma::vec3 *out_normals_src
) const
{
    using std::sqrt;

    if(index != 0)
    {
        // A sphere can only have one span.
        for(; query_src != query_lim;
            ++query_src, ++out_spans_src, out_normals_src += 2)
        {
            *out_spans_src = ballistae::span<double>::nan();
            // Don't need to write any normals, since the span is nan.
        }

        return;
    }

    for(; query_src != query_lim;
        ++query_src, ++out_spans_src, out_normals_src += 2)
    {
        const ballistae::dray3 &query = *query_src;

        auto offset = query.point - this->center;

        auto b = arma::dot(query.slope, offset);
        auto c = arma::dot(offset, offset) - this->radius_squared;

        // We rely on std::sqrt's mandated NaN behavior.
        auto t_min = -b - sqrt(b * b - c);
        auto t_max = -b + sqrt(b * b - c);

        if(ballistae::overlaps(must_overlap, {t_min, t_max}))
        {
            auto point_min = ballistae::eval_ray(query, t_min);
            auto point_max = ballistae::eval_ray(query, t_max);

            arma::vec3 normal_min = arma::normalise(point_min - this->center);
            arma::vec3 normal_max = arma::normalise(point_max - this->center);

            *out_spans_src = {t_min, t_max};
            out_normals_src[0] = normal_min;
            out_normals_src[1] = normal_max;
        }
        else
        {
            out_spans_src[0] = ballistae::span<double>::nan();
        }
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
