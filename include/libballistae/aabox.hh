#ifndef LIBBALLISTAE_AABOX_HH
#define LIBBALLISTAE_AABOX_HH

#include <cmath>

#include <array>
#include <utility>

#include "include/libballistae/ray.hh"
#include "include/libballistae/span.hh"
#include "include/libballistae/vector.hh"

namespace ballistae
{

template<typename Field, size_t D>
struct aabox final
{
    std::array<span<Field>, D> spans;

    span<Field>& operator[](size_t i)
    {
        return spans[i];
    }

    const span<Field>& operator[](size_t i) const
    {
        return spans[i];
    }

    static aabox<Field, D> nan();

    // A bounding box suitable for use as the zero element in a call to
    // std::accumulate with the min_containing aggregator function.
    static aabox<Field, D> accum_zero()
    {
        aabox<Field, D> result;
        for(size_t i = 0; i < D; ++i)
        {
            result[i].lo = std::numeric_limits<Field>::infinity();
            result[i].hi = -std::numeric_limits<Field>::infinity();
        }
        return result;
    }
};

template<typename Field, size_t D>
aabox<Field, D> aabox<Field, D>::nan()
{
    aabox<Field, D> result;
    for(size_t i = 0; i < D; ++i)
        result.spans[i] = span<Field>::nan();
    return result;
}

/// Order A and B along the given axis.
///
/// Convenient for use with std::bind.
template<typename Field, size_t D>
bool aabox_axial_comparator(
    size_t axis,
    const aabox<Field, D> &a,
    const aabox<Field, D> &b
)
{
    return a.spans[axis] < b.spans[axis];
}

/// Smallest aabox containing A and B.
template<typename Field, size_t D>
aabox<Field, D> min_containing(
    const aabox<Field, D> &a,
    const aabox<Field, D> &b
)
{
    aabox<Field, D> result;
    for(size_t i = 0; i < D; ++i)
        result.spans[i] = min_containing(a.spans[i], b.spans[i]);
    return result;
}

/// Grow BOX to include P.
template<typename Field, size_t D>
aabox<Field, D> min_containing(
    const aabox<Field, D> &box,
    const fixvec<Field, D> &p
)
{
    aabox<Field, D> result;
    for(size_t i = 0; i < D; ++i)
        result.spans[i] = min_containing(box[i], p(i));
    return result;
}

/// Cut BOX along AXIS at CUT_PLANE.
///
/// Preconditions:
///
///   * The plane defined by AXIS and CUT_PLANE must make contact with BOX.
template<typename Field, size_t D>
std::array<aabox<Field, D>, 2> cut(
    const aabox<Field, D> &box,
    size_t axis,
    const Field &cut_plane
)
{
    auto span_cut = cut(box.spans[axis], cut_plane);

    aabox<Field, D> lo = box;
    aabox<Field, D> hi = box;
    lo.spans[axis] = span_cut[0];
    hi.spans[axis] = span_cut[1];
    return {lo, hi};
}

template<typename Field, size_t D>
span<Field> ray_test(
    const ray_segment<Field, D> &r,
    const aabox<Field, D> &b
)
{
    using std::swap;

    span<Field> cover = {
        -std::numeric_limits<Field>::infinity(),
        std::numeric_limits<Field>::infinity()
    };

    for(size_t i = 0; i < D; ++i)
    {
        span<double> cur = {
            (b.spans[i].lo - r.the_ray.point(i)) / r.the_ray.slope(i),
            (b.spans[i].hi - r.the_ray.point(i)) / r.the_ray.slope(i)
        };

        if(cur.hi < cur.lo)
            swap(cur.lo, cur.hi);

        if(!(overlaps(cover, cur)))
            return span<double>::nan();

        cover = max_intersecting(cover, cur);
    }

    return overlaps(cover, r.the_segment)
        ? max_intersecting(cover, r.the_segment)
        : span<double>::nan();
}

template<typename Field, size_t D>
Field surface_area(const aabox<Field, D> &box)
{
    Field accum = 0;
    for(size_t axis_a = 0; axis_a < D; ++axis_a)
    {
        for(size_t axis_b = axis_a; axis_b < D; ++axis_b)
        {
            accum += Field(2)
                * measure(box.spans[axis_a])
                * measure(box.spans[axis_b]);
        }
    }

    return accum;
}

template<typename Field, size_t D>
bool isfinite(const aabox<Field, D> &box)
{
    using std::isfinite;

    for(size_t i = 0; i < D; ++i)
    {
        if(!isfinite(measure(box.spans[i])))
            return false;
    }

    return true;
}

/// Get a bounding box that contains a transformed bounding box.
///
/// Note that incremental updates using these grown bounding boxes could cause
/// the bounding box to grow without bound.
template<typename Field, size_t D>
aabox<Field, D> operator*(
    const affine_transform<Field, D> &t,
    const aabox<Field, D> &box
)
{
    auto result = aabox<Field, D>::accum_zero();

    size_t npoints = size_t(1) << D;
    for(size_t i = 0; i < npoints; ++i)
    {
        fixvec<Field, D> cur_point;
        for(size_t j = 0; j < D; ++j)
            cur_point(j) = ((i>>j) & 0x1) ? box[j].hi : box[j].lo;

        result = min_containing(result, t * cur_point);
    }

    return result;
}

}

#endif
