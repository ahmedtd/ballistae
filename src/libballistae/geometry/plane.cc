#include <libballistae/geometry/plane.hh>

#include <cmath>

#include <random>

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/contact.hh>
#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>

using namespace frustum;
using namespace ballistae;

plane::plane()
{
}

plane::~plane()
{
}

aabox<double, 3> plane::get_aabox()
{
    // Very few planes are not infinite on all principle axes.
    aabox<double, 3> infinity = {
        -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()
    };

    return infinity;
}

void plane::crush(const scene &the_scene, double time)
{
    // Nothing to do.
}

contact<double> plane::ray_into(
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

contact<double> plane::ray_exit(
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
