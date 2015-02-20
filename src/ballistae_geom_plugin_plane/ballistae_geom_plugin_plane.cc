#include <libballistae/geom_plugin_interface.hh>
#include <libguile_ballistae/geom_plugin_interface.hh>

#include <cmath>

#include <memory>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/contact.hh>

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

    virtual ballistae::contact ray_intersect(
        const ballistae::dray3 &query
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

ballistae::contact plane_priv::ray_intersect(
    const ballistae::dray3 &query
) const
{
    auto t = arma::dot(center - query.point, normal)
        / arma::dot(query.slope, normal);

    return ballistae::contact(
        t,
        ballistae::eval_ray(query, t),
        normal
    );
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