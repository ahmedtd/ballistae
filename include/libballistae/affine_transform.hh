#ifndef LIBBALLISTAE_AFFINE_TRANSFORM_HH
#define LIBBALLISTAE_AFFINE_TRANSFORM_HH

#include <cmath>

#include <type_traits>

#include <libballistae/vector.hh>

namespace ballistae
{

template<class Field, size_t Dim>
struct affine_transform
{
    fixmat<Field, Dim, Dim> linear;
    fixvec<Field, Dim>      offset;

    static affine_transform<Field, Dim> identity()
    {
        return {fixmat<Field, Dim, Dim>::eye(), fixvec<Field, Dim>::zero()};
    }

    static affine_transform<Field, Dim> translation(
        const fixvec<Field, Dim> &t
    )
    {
        return {fixmat<Field, Dim, Dim>::eye(), t};
    }

    static affine_transform<Field, Dim> scaling(
        const Field &s
    )
    {
        return {fixmat<Field, Dim, Dim>::eye() * s, fixvec<Field, Dim>::zero()};
    }

    /// Generate a rotation transform from an axis-angle representation.
    ///
    /// Restricted to 3-dimensional transforms using std::enable_if.
    static
    typename std::enable_if<Dim == 3, affine_transform<Field, 3>>::type
    rotation(
        const fixvec<Field, 3> &axis,
        const Field &angle
    )
    {
        using std::cos;
        using std::sin;

        const Field s = sin(angle);
        const Field c = cos(angle);

        const Field C = (1 - c);

        const Field &x = axis(0);
        const Field &y = axis(1);
        const Field &z = axis(2);

        affine_transform<Field, 3> result;

        result.linear = {
            x*x*C + c,   x*y*C - z*s, x*z*C + y*s,
            y*x*C + z*s, y*y*C + c,   y*z*C - x*s,
            z*x*C - y*s, z*y*C + x*s, z*z*C + c
        };

        result.offset = {0, 0, 0};

        return result;
    }

    /// Generate a linear mapping from a target basis represented as three
    /// vectors in the source basis.
    static
    typename std::enable_if<Dim == 3, affine_transform<Field, 3> >::type
    basis_mapping(
        const fixvec<Field, 3> &t1,
        const fixvec<Field, 3> &t2,
        const fixvec<Field, 3> &t3
    )
    {
        affine_transform<Field, 3> result;

        result.linear = {
            t1(0), t1(1), t1(2),
            t2(0), t2(1), t2(2),
            t3(0), t3(1), t3(2)
        };

        result.offset = {0, 0, 0};

        return result;
    }
};

template<class Field, size_t Dim>
affine_transform<Field, Dim> operator*(
    const affine_transform<Field, Dim> &a,
    const affine_transform<Field, Dim> &b
)
{
    fixmat<double, 3, 3> linear = a.linear * b.linear;
    fixvec<double, 3> offset = a.offset + a.linear * b.offset;
    return {linear, offset};
}

template<class Field, size_t Dim>
fixvec<Field, Dim> operator*(
    const affine_transform<Field, Dim> &a,
    const fixvec<Field, Dim> &b
)
{
    return a.linear * b + a.offset;
}

template<class Field, size_t Dim>
affine_transform<Field, Dim> inverse(
    const affine_transform<Field, Dim> &t
)
{
    auto total = fixmat<Field, Dim+1, Dim+1>::eye();

    for(size_t i = 0; i < Dim; ++i)
        for(size_t j = 0; j < Dim; ++j)
            total(i, j) = t.linear(i, j);

    for(size_t i = 0; i < Dim; ++i)
        total(i, Dim) = t.offset(i);

    fixmat<Field, Dim+1, Dim+1> inv_total = inv(total);

    affine_transform<Field, Dim> result;

    for(size_t i = 0; i < Dim; ++i)
        for(size_t j = 0; j < Dim; ++j)
            result.linear(i, j) = inv_total(i, j);

    for(size_t i = 0; i < Dim; ++i)
        result.offset(i) = inv_total(i, Dim);

    return result;
}

}

#endif
