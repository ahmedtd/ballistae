#include <libballistae/geometry/box.hh>

namespace ballistae
{

box::box(std::array<span<double>, 3> spans_in)
    : spans(spans_in)
{
}

box::~box()
{
}

aabox<double, 3> box::get_aabox()
{
    aabox<double, 3> bounds = {spans};
    return bounds;
}

void box::crush(double time)
{
}

contact<double> box::ray_into(
    const ray_segment<double,3> &query
) const
{
    using std::max;
    using std::min;
    using std::swap;

    span<double> cover = {
        -std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::infinity()
    };

    fixvec<double, 3> hit_axis;
    for(size_t i = 0; i < 3; ++i)
    {
        span<double> cur = {
            (spans[i].lo - query.the_ray.point(i)) / query.the_ray.slope(i),
            (spans[i].hi - query.the_ray.point(i)) / query.the_ray.slope(i)
        };

        double normal_component = -1;
        if(cur.hi < cur.lo)
        {
            swap(cur.lo, cur.hi);
            normal_component = 1;
        }

        if(!(overlaps(cover, cur)))
            return contact<double>::nan();

        if(cover.lo < cur.lo)
        {
            cover.lo = cur.lo;
            hit_axis = {0, 0, 0};
            hit_axis[i] = normal_component;
        }

        if(cur.hi < cover.hi)
        {
            cover.hi = cur.hi;
        }

    }

    if(overlaps(cover, query.the_segment))
    {
        contact<double> result;

        result.t = max(cover.lo, query.the_segment.lo);
        result.r = query.the_ray;
        result.p = eval_ray(query.the_ray, result.t);
        result.n = hit_axis;
        result.mtl2 = {0, 0};
        result.mtl3 = {result.p};

        return result;
    }
    else
    {
        return contact<double>::nan();
    }
}

contact<double> box::ray_exit(
    const ray_segment<double,3> &query
) const
{
    using std::max;
    using std::min;
    using std::swap;

    span<double> cover = {
        -std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::infinity()
    };

    fixvec<double, 3> hit_axis;
    for(size_t i = 0; i < 3; ++i)
    {
        span<double> cur = {
            (spans[i].lo - query.the_ray.point(i)) / query.the_ray.slope(i),
            (spans[i].hi - query.the_ray.point(i)) / query.the_ray.slope(i)
        };

        double normal_component = 1;
        if(cur.hi < cur.lo)
        {
            swap(cur.lo, cur.hi);
            normal_component = -1;
        }

        if(!(overlaps(cover, cur)))
            return contact<double>::nan();

        if(cover.lo < cur.lo)
        {
            cover.lo = cur.lo;
        }

        if(cur.hi < cover.hi)
        {
            cover.hi = cur.hi;
            hit_axis = {0, 0, 0};
            hit_axis[i] = normal_component;
        }

    }

    if(overlaps(cover, query.the_segment))
    {
        contact<double> result;

        result.t = min(cover.hi, query.the_segment.hi);
        result.r = query.the_ray;
        result.p = eval_ray(query.the_ray, result.t);
        result.n = hit_axis;
        result.mtl2 = {0, 0};
        result.mtl3 = {result.p};

        return result;
    }
    else
    {
        return contact<double>::nan();
    }
}

}
