#include <libballistae/geometry/infinity.hh>

#include <cmath>

#include <limits>

#include <frustum-0/indicial/fixed.hh>

using namespace frustum;
using namespace ballistae;

aabox<double, 3> infinity::get_aabox()
{
    // What were you expecting?
    aabox<double, 3> infinity = {
        -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()
    };

    return infinity;
}

void infinity::crush(const scene &the_scene, double time)
{
    // Nothing to do.
}

contact<double> infinity::ray_into(
    const scene &the_scene,
    const ray_segment<double,3> &query
) const
{
    using std::acos;
    using std::atan2;

    auto infty = std::numeric_limits<double>::infinity();

    if(query.the_segment.hi() == infty)
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

contact<double> infinity::ray_exit(
    const scene &the_scene,
    const ray_segment<double,3> &query
) const
{
    using std::acos;
    using std::atan2;

    auto infty = std::numeric_limits<double>::infinity();

    if(query.the_segment.lo() == -infty)
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
