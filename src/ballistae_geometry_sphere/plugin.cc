#include <libballistae/geometry.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>

#include <cmath>

#include <random>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <frustum-0/indicial/fixed.hh>
#include <libguile_frustum0/libguile_frustum0.hh>

#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

using namespace frustum;
using namespace ballistae;

class sphere_priv : public geometry
{
public:
    sphere_priv();

    virtual ~sphere_priv();

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

sphere_priv::sphere_priv()
{
}

sphere_priv::~sphere_priv()
{
}

contact<double> sphere_priv::ray_into(
    const scene &the_scene,
    const ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    using std::acos;
    using std::atan2;
    using std::sqrt;

    auto b = iprod(query.the_ray.slope, query.the_ray.point);
    auto c = iprod(query.the_ray.point, query.the_ray.point) - 1.0;

    // We rely on std::sqrt's mandated NaN behavior.
    auto t_min = -b - sqrt(b * b - c);

    if(contains(query.the_segment, t_min))
    {
        contact<double> result;

        auto p = eval_ray(query.the_ray, t_min);
        result.t = t_min;
        result.p = p;
        result.n = normalise(p);
        result.mtl2 = {atan2(p(0), p(1)), acos(p(2))};
        result.mtl3 = p;
        result.r = query.the_ray;

        return result;
    }
    else
    {
        return contact<double>::nan();
    }
}

contact<double> sphere_priv::ray_exit(
    const scene &the_scene,
    const ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    using std::acos;
    using std::atan2;
    using std::sqrt;

    auto b = iprod(query.the_ray.slope, query.the_ray.point);
    auto c = iprod(query.the_ray.point, query.the_ray.point) - 1.0;

    // We rely on std::sqrt's mandated NaN behavior.
    auto t_max = -b + sqrt(b * b - c);

    if(contains(query.the_segment, t_max))
    {
        contact<double> result;
        auto p = eval_ray(query.the_ray, t_max);
        result.t = t_max;
        result.p = p;
        result.n = normalise(p);
        result.mtl2 = {atan2(p(0), p(1)), acos(p(2))};
        result.mtl3 = p;
        result.r = query.the_ray;

        return result;
    }
    else
    {
        return contact<double>::nan();
    }
}

std::unique_ptr<geometry> guile_ballistae_geometry(
    scene *p_scene,
    SCM config_alist
)
{
    return std::make_unique<sphere_priv>();
}
