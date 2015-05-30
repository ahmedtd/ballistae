#ifndef LIBBALLISTAE_CONTACT_HH
#define LIBBALLISTAE_CONTACT_HH

#include <frustum-0/geometry/affine_transform.hh>

#include <libballistae/ray.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

template<class Field>
struct contact final
{
    Field t;
    ray<Field, 3>    r;
    fixvec<Field, 3> p;
    fixvec<Field, 3> n;
    fixvec<Field, 2> mtl2;
    fixvec<Field, 3> mtl3;

    static contact<Field> nan()
    {
        constexpr Field nan = std::numeric_limits<Field>::quiet_NaN();
        contact result;
        result.t = nan;
        return result;
    }
};

/// Apply an affine transform to a contact.
///
/// We require the transpose inverse of the linear part of the transform.
/// Rather than calculate it every time, we take it as an argument.
template<class Field>
contact<Field> contact_transform(
    const contact<Field> &c,
    const affine_transform<Field, 3> &t,
    const fixmat<Field, 3, 3> &nm
)
{
    contact<Field> result = c;

    // We don't use the standard ray-transforming support, since we
    // need to know how the transform changes the scale of the
    // underlying space.
    result.r.point = t * result.r.point;
    result.r.slope = t.linear * result.r.slope;

    // Assume that the incoming ray has normalized slope.
    double scale_factor = norm(result.r.slope);
    result.r.slope /= scale_factor;
    result.t *= scale_factor;

    result.p = t * result.p;
    result.n = normalise(nm * result.n);
    return result;
}

}

#endif
