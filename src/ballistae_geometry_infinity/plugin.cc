#include <libballistae/geometry.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>

#include <cmath>

#include <limits>
#include <memory>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

#include <libguile_armadillo/libguile_armadillo.hh>

namespace bl = ballistae;

class infty_priv final : public ballistae::geometry
{
public:
    virtual bl::contact<double> ray_into(
        const bl::scene &the_scene,
        const bl::ray_segment<double,3> &query,
        std::ranlux24 &thread_rng
    ) const;

    virtual bl::contact<double> ray_exit(
        const bl::scene &the_scene,
        const bl::ray_segment<double,3> &query,
        std::ranlux24 &thread_rng
    ) const;
};

bl::contact<double> infty_priv::ray_into(
    const bl::scene &the_scene,
    const bl::ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    using std::acos;
    using std::atan2;

    auto infty = std::numeric_limits<double>::infinity();

    if(query.the_segment.hi == infty)
    {
        bl::fixvec<double, 3> p = bl::eval_ray(query.the_ray, infty);
        bl::contact<double> result = {
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
        return bl::contact<double>::nan();
    }
}

bl::contact<double> infty_priv::ray_exit(
    const bl::scene &the_scene,
    const bl::ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    auto infty = std::numeric_limits<double>::infinity();

    if(query.the_segment.lo == -infty)
    {
        bl::fixvec<double, 3> p = bl::eval_ray(query.the_ray, infty);
        bl::contact<double> result = {
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
        return bl::contact<double>::nan();
    }
}

ballistae::geometry* guile_ballistae_geometry(
    SCM config_alist
)
{
    return new infty_priv();
}
