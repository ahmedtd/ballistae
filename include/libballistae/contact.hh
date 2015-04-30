#ifndef LIBBALLISTAE_CONTACT_HH
#define LIBBALLISTAE_CONTACT_HH

#include <libballistae/affine_transform.hh>
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
    fixvec<Field, 2> uv;
    fixvec<Field, 3> uvw;

    static contact<Field> nan()
    {
        constexpr Field nan = std::numeric_limits<Field>::quiet_NaN();
        contact result;
        result.t = nan;
        return result;
    }
};

template<class Field>
contact<Field> operator*(
    const affine_transform<Field, 3> &t,
    const contact<Field> &c
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

    result.p = t * result.p;
    result.n = normalise(t.linear * result.n);
    result.t *= scale_factor;
    return result;
}

}

#endif
