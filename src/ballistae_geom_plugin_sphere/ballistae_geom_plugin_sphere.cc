#include <libballistae/geom_plugin_interface.hh>
#include <libguile_ballistae/geom_plugin_interface.hh>

#include <cmath>

#include <memory>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/contact.hh>

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

    virtual ballistae::contact ray_intersect(
        const ballistae::dray3 &query
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

ballistae::contact sphere_priv::ray_intersect(
    const ballistae::dray3 &query
) const
{
    using std::sqrt;

    auto b = arma::dot(query.slope, query.point - this->center);
    auto c = arma::dot(query.point - this->center, query.point - this->center)
        - this->radius_squared;

    // We rely on std::sqrt's mandated NaN behavior.
    auto t = -b - sqrt(b * b - c);
    auto point = eval_ray(query, t);
    auto normal = arma::normalise(point - this->center);

    return ballistae::contact(
        t,
        point,
        normal
    );
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
