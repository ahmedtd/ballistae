#include <libballistae/geometry.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>

#include <cmath>

#include <random>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <frustum-0/indicial/fixed.hh>
#include <libguile_frustum0/libguile_frustum0.hh>

#include <libballistae/contact.hh>
#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

using namespace frustum;
using namespace ballistae;

class cylinder_priv : public ballistae::geometry
{
public:
    fixvec<double, 3> center;
    fixvec<double, 3> axis;
    double radius_squared;

public:
    cylinder_priv(
        const fixvec<double, 3> &center_in,
        const fixvec<double, 3> &axis_in,
        const double &radius_in
    );

    virtual ~cylinder_priv();

    virtual contact<double> ray_into(
        const scene &the_scene,
        const ray_segment<double,3> &query,
        std::ranlux24 &thread_rng
    ) const;

    virtual contact<double> ray_exit(
        const scene &the_scene,
        const ray_segment<double,3> &query,
        std::ranlux24 &thread_rng
    ) const;
};

cylinder_priv::cylinder_priv(
    const fixvec<double, 3> &center_in,
    const fixvec<double, 3> &axis_in,
    const double &radius_in
)
    : center(center_in),
      axis(normalise(axis_in)),
      radius_squared(radius_in * radius_in)
{
}

cylinder_priv::~cylinder_priv()
{
}

contact<double> cylinder_priv::ray_into(
    const scene &the_scene,
    const ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    using std::atan2;
    using std::sqrt;

    auto foil_a = reject(axis, query.the_ray.slope);
    auto foil_b = reject(axis, query.the_ray.point - center);

    double a = iprod(foil_a, foil_a);
    double b = 2.0 * iprod(foil_a, foil_b);
    double c = iprod(foil_b, foil_b) - radius_squared;

    double t_min = (-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a);

    if(!std::isnan(t_min))
    {
        // Ray hits cylinder.

        if(contains(query.the_segment, t_min))
        {
            contact<double> result;

            result.t = t_min;
            result.r = query.the_ray;
            result.p = eval_ray(query.the_ray, t_min);
            result.n = reject(axis, result.p - center);
            result.mtl2 = {result.p(0), std::atan2(result.p(1), result.p(2))};
            result.mtl3 = result.p;

            return result;
        }
        else
        {
            return contact<double>::nan();
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
                contact<double> result;
                result.t = -std::numeric_limits<double>::infinity();
                result.r = query.the_ray;
                return result;
            }
            else
                return contact<double>::nan();
        }
    }
}

contact<double> cylinder_priv::ray_exit(
    const scene &the_scene,
    const ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    using std::atan2;
    using std::sqrt;

    auto foil_a = reject(axis, query.the_ray.slope);
    auto foil_b = reject(axis, query.the_ray.point - center);

    double a = iprod(foil_a, foil_a);
    double b = 2.0 * iprod(foil_a, foil_b);
    double c = iprod(foil_b, foil_b) - radius_squared;

    double t_max = (-b + sqrt(b * b - 4.0 * a * c)) / (2.0 * a);

    if(!std::isnan(t_max))
    {
        // The ray hits the cylinder.

        if(contains(query.the_segment, t_max))
        {
            contact<double> result;

            result.t = t_max;
            result.r = query.the_ray;
            result.p = eval_ray(query.the_ray, t_max);
            result.n = reject(axis, result.p - center);
            result.mtl2 = {result.p(0), std::atan2(result.p(1), result.p(2))};
            result.mtl3 = result.p;

            return result;
        }
        else
        {
            return contact<double>::nan();
        }
    }
    else
    {
        // Ray misses cylinder.  It could miss inside or outside.

        if(contains(query.the_segment, std::numeric_limits<double>::infinity()))
        {
            contact<double> result;
            result.t = std::numeric_limits<double>::infinity();
            result.r = query.the_ray;
            return result;
        }
        else
            return contact<double>::nan();
    }
}

std::unique_ptr<geometry> guile_ballistae_geometry(
    scene *p_scene,
    SCM config_alist
)
{
    SCM sym_center = scm_from_utf8_symbol("center");
    SCM sym_axis   = scm_from_utf8_symbol("axis");
    SCM sym_radius = scm_from_utf8_symbol("radius");

    auto cyl_p = std::make_unique<cylinder_priv>(
        fixvec<double, 3>{0, 0, 0},
        fixvec<double, 3>{0, 0, 1},
        1.0
    );

    SCM center_lookup = scm_assq_ref(config_alist, sym_center);
    SCM axis_lookup   = scm_assq_ref(config_alist, sym_axis);
    SCM radius_lookup = scm_assq_ref(config_alist, sym_radius);

    if(scm_is_true(center_lookup))
    {
        cyl_p->center = guile_frustum::dvec3_from_scm(center_lookup);
    }

    if(scm_is_true(axis_lookup))
    {
        cyl_p->axis = normalise(guile_frustum::dvec3_from_scm(center_lookup));
    }

    if(scm_is_true(radius_lookup))
    {
        cyl_p->radius_squared = scm_to_double(radius_lookup);
    }

    return std::move(cyl_p);
}
