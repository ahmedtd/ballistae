#include <libballistae/geometry.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>

#include <cmath>

#include <limits>
#include <memory>

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

class infty_priv final : public geometry
{
public:
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

contact<double> infty_priv::ray_into(
    const scene &the_scene,
    const ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    using std::acos;
    using std::atan2;

    auto infty = std::numeric_limits<double>::infinity();

    if(query.the_segment.hi == infty)
    {
        fixvec<double, 3> p = eval_ray(query.the_ray, infty);
        contact<double> result = {
            infty,
            query.the_ray,
            p,
            -query.the_ray.slope,
            {
                atan2(query.the_ray.slope(0), query.the_ray.slope(1)),
                acos(query.the_ray.slope(2))
            },
            p
        };
        return result;
    }
    else
    {
        return contact<double>::nan();
    }
}

contact<double> infty_priv::ray_exit(
    const scene &the_scene,
    const ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    using std::acos;
    using std::atan2;

    auto infty = std::numeric_limits<double>::infinity();

    if(query.the_segment.lo == -infty)
    {
        fixvec<double, 3> p = eval_ray(query.the_ray, infty);
        contact<double> result = {
            -infty,
            query.the_ray,
            p,
            query.the_ray.slope,
            {
                atan2(-query.the_ray.slope(0), -query.the_ray.slope(1)),
                acos(-query.the_ray.slope(2))
            },
            p
        };
        return result;
    }
    else
    {
        return contact<double>::nan();
    }
}

geometry* guile_ballistae_geometry(
    SCM config_alist
)
{
    return new infty_priv();
}
