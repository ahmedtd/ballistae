#include <libballistae/geometry.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>

#include <cmath>

#include <random>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/contact.hh>
#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>

#include <libguile_armadillo/libguile_armadillo.hh>

namespace bl = ballistae;

class plane_priv : public ballistae::geometry
{
public:
    plane_priv();
    virtual ~plane_priv();

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

plane_priv::plane_priv()
{
}

plane_priv::~plane_priv()
{
}

bl::contact<double> plane_priv::ray_into(
    const bl::scene &the_scene,
    const bl::ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    auto height = query.the_ray.point(0);
    auto slope = query.the_ray.slope(0);
    auto t = -height / slope;

    if(slope < double(0) && contains(query.the_segment, t))
    {
        bl::contact<double> result;

        result.t = t;
        result.r = query.the_ray;
        result.p = ballistae::eval_ray(query.the_ray, t);
        result.n = {1, 0, 0};
        result.uv = {result.p(1), result.p(2)};
        result.uvw = result.p;

        return result;
    }
    else
    {
        // Ray never intersects plane.
        return bl::contact<double>::nan();
    }
}

bl::contact<double> plane_priv::ray_exit(
    const bl::scene &the_scene,
    const bl::ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    auto height = query.the_ray.point(0);
    auto slope = query.the_ray.slope(0);
    auto t = -height / slope;

    if(slope > double(0) && contains(query.the_segment, t))
    {
        bl::contact<double> result;

        result.t = t;
        result.r = query.the_ray;
        result.p = ballistae::eval_ray(query.the_ray, t);
        result.n = {1, 0, 0};
        result.uv = {result.p(1), result.p(2)};
        result.uvw = result.p;

        return result;
    }
    else
    {
        // Ray never intersects plane.
        return bl::contact<double>::nan();
    }
}

ballistae::geometry* guile_ballistae_geometry(SCM config_alist)
{
    return new plane_priv();
}
