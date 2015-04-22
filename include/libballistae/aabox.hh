#ifndef LIBBALLISTAE_AABOX_HH
#define LIBBALLISTAE_AABOX_HH

#include <array>
#include <utility>

#include <libballistae/ray.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

template<typename Field, size_t D>
struct aabox final
{
    std::array<span<Field>, D> spans;

    static aabox<Field, D> nan()
    {
        aabox<Field, D> result;
        for(size_t i = 0; i < D; ++i)
            result.spans[i] = span<Field>::nan();
        return result;
    }
};

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
span<double> ray_test(
    const ray_segment<Field, D> &r,
    const aabox<Field, D> &b)
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

        if(!(overlaps(r.the_segment, cur)))
            return span<double>::nan();

        cover = max_intersecting(cover, cur);
    }

    return overlaps(cover, r.the_segment)
        ? cover
        : span<double>::nan();
}

}

#endif
