#include <libballistae/geom_plugin_interface.hh>
#include <libguile_ballistae/geom_plugin_interface.hh>

#include <cmath>

#include <memory>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/contact.hh>
#include <libballistae/vector.hh>

#include <libguile_armadillo/libguile_armadillo.hh>

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

    virtual ballistae::contact ray_intersect(
        const ballistae::dray3 &query
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

ballistae::contact cylinder_priv::ray_intersect(
    const ballistae::dray3 &query
) const
{
    using std::sqrt;

    arma::vec3 foil_a = query.slope - axis * arma::dot(query.slope, axis);
    arma::vec3 foil_b = query.point - center
        - axis * arma::dot(query.point - center, axis);

    double a = arma::dot(foil_a, foil_a);
    double b = 2.0 * arma::dot(foil_a, foil_b);
    double c = arma::dot(foil_b, foil_b) - radius_squared;

    double t = (-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a);

    arma::vec3 p = ballistae::eval_ray(query, t);

    arma::vec3 n = arma::normalise(ballistae::reject(axis, p - center));

    return ballistae::contact(t, p, n);
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
