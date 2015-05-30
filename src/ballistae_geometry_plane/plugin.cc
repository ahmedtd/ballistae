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

using namespace frustum;
using namespace ballistae;

class plane_priv : public geometry
{
public:
    plane_priv();
    virtual ~plane_priv();

    virtual aabox<double, 3> get_aabox();

    virtual void crush(const scene &the_scene, double time);

    virtual contact<double> ray_into(
        const scene &the_scene,
        const ray_segment<double,3> &query
    ) const;

    virtual contact<double> ray_exit(
        const scene &the_scene,
        const ray_segment<double,3> &query
    ) const;
};

plane_priv::plane_priv()
{
}

plane_priv::~plane_priv()
{
}

aabox<double, 3> plane_priv::get_aabox()
{
    // Very few planes are not infinite on all principle axes.
    aabox<double, 3> infinity = {
        -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()
    };

    return infinity;
}

void plane_priv::crush(const scene &the_scene, double time)
{
    // Nothing to do.
}

contact<double> plane_priv::ray_into(
    const scene &the_scene,
    const ray_segment<double,3> &query
) const
{
    auto height = query.the_ray.point(0);
    auto slope = query.the_ray.slope(0);
    auto t = -height / slope;

    if(slope < double(0) && contains(query.the_segment, t))
    {
        contact<double> result;

        result.t = t;
        result.r = query.the_ray;
        result.p = eval_ray(query.the_ray, t);
        result.n = {1, 0, 0};
        result.mtl2 = {result.p(1), result.p(2)};
        result.mtl3 = result.p;

        return result;
    }
    else
    {
        // Ray never intersects plane.
        return contact<double>::nan();
    }
}

contact<double> plane_priv::ray_exit(
    const scene &the_scene,
    const ray_segment<double,3> &query
) const
{
    auto height = query.the_ray.point(0);
    auto slope = query.the_ray.slope(0);
    auto t = -height / slope;

    if(slope > double(0) && contains(query.the_segment, t))
    {
        contact<double> result;

        result.t = t;
        result.r = query.the_ray;
        result.p = eval_ray(query.the_ray, t);
        result.n = {1, 0, 0};
        result.mtl2 = {result.p(1), result.p(2)};
        result.mtl3 = result.p;

        return result;
    }
    else
    {
        // Ray never intersects plane.
        return contact<double>::nan();
    }
}

std::unique_ptr<geometry> guile_ballistae_geometry(
    scene *p_scene,
    SCM config_alist
)
{
    return std::make_unique<plane_priv>();
}
