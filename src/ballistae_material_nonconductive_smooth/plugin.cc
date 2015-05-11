#include <libballistae/material.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

#include <cmath>

#include <random>
#include <utility>

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/dense_signal.hh>
#include <libballistae/vector.hh>

using namespace frustum;
using namespace ballistae;

class nc_smooth : public ballistae_guile::updatable_material
{
public:

    dense_signal<double> n_interior;
    dense_signal<double> n_exterior;

public:

    virtual ~nc_smooth();

    virtual shade_info<double> shade(
        const scene &the_scene,
        const contact<double> &glb_contact,
        double lambda_src,
        double lambda_lim,
        double lambda_cur,
        size_t sample_index,
        std::ranlux24 &thread_rng
    ) const;

    virtual void guile_update(scene *p_scene, SCM config);
};

nc_smooth::~nc_smooth()
{
}

void nc_smooth::guile_update(scene *p_scene, SCM config)
{
    using namespace ballistae_guile;

    SCM sym_n_interior = scm_from_utf8_symbol("n-interior");
    SCM sym_n_exterior = scm_from_utf8_symbol("n-exterior");

    SCM lu_n_interior = scm_assq_ref(config, sym_n_interior);
    SCM lu_n_exterior = scm_assq_ref(config, sym_n_exterior);

    if(scm_is_true(lu_n_interior))
        this->n_interior = signal_from_scm(lu_n_interior);

    if(scm_is_true(lu_n_exterior))
        this->n_exterior = signal_from_scm(lu_n_exterior);
}

shade_info<double> nc_smooth::shade(
    const scene &the_scene,
    const contact<double> &glb_contact,
    double lambda_src,
    double lambda_lim,
    double lambda_cur,
    size_t sample_index,
    std::ranlux24 &thread_rng
) const
{
    using std::pow;
    using std::sqrt;
    using std::swap;

    fixvec<double, 3> p = glb_contact.p;
    fixvec<double, 3> n = glb_contact.n;
    fixvec<double, 3> refl = glb_contact.r.slope;

    double a_cos = iprod(refl, n);

    double n_a = interpolate(n_exterior, lambda_cur);
    double n_b = interpolate(n_interior, lambda_cur);

    if(a_cos > 0.0)
    {
        swap(n_a, n_b);
    }

    double n_r = n_a / n_b;

    // Now the problem is regularized.  We have a ray that was emitted into
    // region A from a planar boundary.  There are two incident rays that put
    // power into this ray:
    //
    //   * One shone on the boundary from region A, and was partially reflected
    //     into the ray we have.
    //
    //   * One shone on the boundary from region B, and was partially
    //     transmitted into the ray we have.

    double snell = 1 - pow(n_r, 2) * (1.0 - pow(a_cos, 2));

    if(snell < 0.0)
    {
        // All power was contributed by the reflected ray (total internal
        // reflection).

        shade_info<double> result;
        result.emitted_power = 0;
        result.propagation_k = 1.0;
        result.incident_ray.point = p;
        result.incident_ray.slope = reflect(refl, n);
        return result;
    }

    double b_cos = (a_cos < 0.0) ? -sqrt(snell) : sqrt(snell);

    double ab = n_r * b_cos / a_cos;
    double ab_i = 1.0 / ab;

    // We are solving the reverse problem, so the coefficient of transmission
    // must actually be calculated from the perspective of the ray shining on
    // the boundary from region B.
    double coeff_refl = pow((1 - ab) / (1 + ab), 2);
    double coeff_tran = ab_i * pow(2 / (1 + ab_i), 2);

    std::uniform_real_distribution<> dist(0, coeff_refl + coeff_tran);

    shade_info<double> result;
    result.emitted_power = 0.0;
    if(dist(thread_rng) < coeff_refl)
    {
        // Give the ray that contributed by reflection.
        result.propagation_k = 1;
        result.incident_ray.slope = reflect(refl, n);
    }
    else
    {
        // Give the ray that contributed by refraction.
        result.propagation_k = 1;
        result.incident_ray.slope = (b_cos - n_r * a_cos) * n + n_r * refl;
    }

    result.incident_ray.point = p;

    return result;
}

ballistae_guile::updatable_material*
guile_ballistae_material(scene *p_scene, SCM config)
{
    nc_smooth *p = new nc_smooth();

    p->n_interior = pulse<double>(390, 835, 1, 390, 835, 1.0);
    p->n_exterior = pulse<double>(390, 835, 1, 390, 835, 1.0);

    p->guile_update(p_scene, config);

    return p;
}
