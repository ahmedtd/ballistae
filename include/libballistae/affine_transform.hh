#ifndef LIBBALLISTAE_AFFINE_TRANSFORM_HH
#define LIBBALLISTAE_AFFINE_TRANSFORM_HH

#include <cmath>

#include <armadillo>

#include <libballistae/ray.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

template<class Field, size_t Dim>
struct affine_transform
{
    affine_transform()
        : linear(arma::fill::eye),
          offset(arma::fill::zeros)
    {
    }

    affine_transform(
        const fixmat<double, 3, 3> &linear_in,
        const fixvec<double, 3>    &offset_in
    )
        : linear(linear_in),
          offset(offset_in)
    {
    }

    affine_transform(const affine_transform &other) = default;
    affine_transform(affine_transform &&other) = default;

    affine_transform<Field, Dim>& operator=(const affine_transform<Field, Dim> &other) = default;
        affine_transform<Field, Dim>& operator=(affine_transform<Field, Dim> &other) = default;

        fixmat<Field, Dim, Dim> linear;
    fixvec<Field, Dim>      offset;

};

// TODO: Rewrite using return type synthesis once we don't need gcc 4.7.

template<class Field, size_t Dim>
affine_transform<Field, Dim> operator*(
    const affine_transform<Field, Dim> &a,
    const affine_transform<Field, Dim> &b
)
{
    fixmat<double, 3, 3> linear = a.linear * b.linear;
    fixvec<double, 3> offset = a.offset + a.linear * b.offset;
    return affine_transform<Field, Dim>(linear, offset);
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
ray<Field, Dim> operator*(
    const affine_transform<Field, Dim> &a,
    const ray<Field, Dim> &b
)
{
    return {
        a.linear * b.point + a.offset,
        arma::normalise(a.linear * b.slope)
    };
}

template<class Field, size_t Dim>
affine_transform<Field, Dim> inverse(
    const affine_transform<Field, Dim> &a
)
{
    return {arma::inv(a.linear), -a.offset};
}

template<class Field, size_t Dim>
affine_transform<Field, Dim> translation(
    const fixvec<Field, Dim> &t
)
{
    affine_transform<Field, Dim> result;
    result.offset = t;
    return result;
}

template<class Field, size_t Dim>
affine_transform<Field, Dim> scaling(
    const Field &s
)
{
    affine_transform<Field, Dim> result;
    result.linear *= s;
    return result;
}

/// Generate a rotation transform from an axis-angle representation.
template<class Field>
affine_transform<Field, 3> rotation(
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
    result.linear = arma::trans(result.linear);

    result.offset = {0, 0, 0};

    return result;
}

/// Generate a linear mapping from a target basis represented as three
/// vectors in the source basis.
template<class Field>
affine_transform<Field, 3> basis_mapping(
    const fixvec<Field, 3> &t1,
    const fixvec<Field, 3> &t2,
    const fixvec<Field, 3> &t3
)
{
    affine_transform<Field, 3> result;

    // Armadillo uses column-major layout.
    result.linear = {
        t1(0), t1(1), t1(2),
        t2(0), t2(1), t2(2),
        t3(0), t3(1), t3(2)
    };
    result.linear = arma::trans(result.linear);

    result.offset = {0, 0, 0};

    return result;
}

}

#endif
