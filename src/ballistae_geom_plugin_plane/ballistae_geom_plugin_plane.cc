#include <libballistae/geom_plugin_interface.hh>
#include <libguile_ballistae/geom_plugin_interface.hh>

#include <cmath>

#include <memory>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>

#include <libguile_armadillo/libguile_armadillo.hh>

class plane_priv : public ballistae::geom_priv
{
public:
    arma::vec3 center;
    arma::vec3 normal;

    plane_priv(
        const arma::vec3 &center_in,
        const arma::vec3 &normal_in
    );

    virtual ~plane_priv();

    virtual void ray_intersect(
        const ballistae::scene &the_scene,
        const ballistae::dray3 *query_src,
        const ballistae::dray3 *query_lim,
        const ballistae::span<double> &must_overlap,
        const std::size_t index,
        ballistae::span<double> *out_spans_src,
        arma::vec3 *out_normals_src
    ) const;
};

plane_priv::plane_priv(
    const arma::vec3 &center_in,
    const arma::vec3 &normal_in
)
    : center(center_in),
      normal(arma::normalise(normal_in))
{
}

plane_priv::~plane_priv()
{
}

void plane_priv::ray_intersect(
    const ballistae::scene &the_scene,
    const ballistae::dray3 *query_src,
    const ballistae::dray3 *query_lim,
    const ballistae::span<double> &must_overlap,
    const std::size_t index,
    ballistae::span<double> *out_spans_src,
    arma::vec3 *out_normals_src
) const
{
    constexpr auto infty = std::numeric_limits<double>::infinity();

    if(index != 0)
    {
        // A plane can only have one span.
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
        auto height = arma::dot(center - query_src->point, normal);
        auto slope = arma::dot(query_src->slope, normal);
        auto t = height / slope;

        if(slope > double(0))
        {
            if(overlaps(must_overlap, {-infty, t}))
            {
                out_spans_src[0] = {
                    -std::numeric_limits<double>::infinity(),
                    t
                };

                // There is no normal at infinity.
                out_normals_src[1] = normal;
            }
            else
            {
                out_spans_src[0] = ballistae::span<double>::nan();
            }
        }
        else if(slope < double(0))
        {
            if(overlaps(must_overlap, {t, infty}))
            {
                out_spans_src[0] = {
                    t,
                    std::numeric_limits<double>::infinity()
                };

                out_normals_src[0] = normal;
                // There is no normal at infinity.
            }
            else
            {
                out_spans_src[0] = ballistae::span<double>::nan();
            }
        }
        else
        {
            // Ray never intersects plane.
            out_spans_src[0] = ballistae::span<double>::nan();
        }
    }
}

std::shared_ptr<ballistae::geom_priv> ballistae_geom_create_from_alist(
    SCM config_alist
)
{
    constexpr const char* const subr = "ballistae_geom_create_from_alist(plane)";

    SCM sym_center = scm_from_utf8_symbol("center");
    SCM sym_normal = scm_from_utf8_symbol("normal");

    SCM cur_head = config_alist;
    while(! scm_is_null(cur_head))
    {
        SCM cur_key = scm_caar(cur_head);
        SCM cur_val = scm_cdar(cur_head);
        cur_head = scm_cdr(cur_head);

        if(scm_is_true(scm_eq_p(sym_center, cur_key))
           || scm_is_true(scm_eq_p(sym_normal, cur_key)))
        {
            SCM_ASSERT_TYPE(
                scm_is_true(
                    arma_guile::generic_col_dim_p<double>(
                        cur_val,
                        scm_from_int(3)
                    )
                ),
                scm_cons(cur_key, cur_val),
                SCM_ARGn,
                subr,
                "arma/b64col[3]"
            );
        }
        else
        {
            scm_wrong_type_arg_msg(subr, SCM_ARGn, cur_key, "Unknown key.");
        }
    }

    // No guile errors below this point.

    arma::vec3 center(arma::fill::zeros);
    arma::vec3 normal(arma::fill::zeros);

    SCM center_lookup = scm_assq_ref(config_alist, sym_center);
    SCM normal_lookup = scm_assq_ref(config_alist, sym_normal);

    if(scm_is_true(center_lookup))
    {
        auto col_p = arma_guile::smob_get_data<arma::Col<double>*>(
            center_lookup
        );
        center = *col_p;
    }

    if(scm_is_true(normal_lookup))
    {
        auto col_p = arma_guile::smob_get_data<arma::Col<double>*>(
            normal_lookup
        );
        normal = *col_p;
    }

    return std::make_shared<plane_priv>(center, normal);
}
