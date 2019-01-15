#ifndef LIBFRUSTUM_AFFINE_TRANSFORM_HH
#define LIBFRUSTUM_AFFINE_TRANSFORM_HH

#include <cmath>

#include <array>

#include <frustum-0/indicial/fixed.hh>

namespace frustum
{

/// A fixed-dimensional affine transform.
///
/// Decomposed into a Dim-dimensional linear mapping and a Dim-dimensional
/// offset.  Operator overloading allows client code to use it as if it were
/// represented using homogeneous coordinates.  For example,
///
///   * Given two transforms A and B, A*B is the left-composition of A and B.
///
///   * Given a transform A and a vector b (with the same Dim parameter), A*b is
///     the result of applying A to b, OFFSET INCLUDED.  If you don't want to
///     include the offset (transforming vectors, not points), then you would
///     write A.linear * b.
///
///   * Given a transform A, inverse(A) computes the inverse transform.
///
/// Some canonical transform constructors are provided:
///
///   * Identity transform.
///
///   * Translation by a vector.
///
///   * Isotropic scaling.
///
///   * Anisotropic scaling on principle axes.
///
///   * 3D rotation, axis-angle representation.
///
///   * Generalized basis-vector mapping.
///
/// Additional operations:
///
///   * Given a transform A, normal_linear_map(A) is the proper linear map to
///   * apply to normal vectors of objects transformed under A.
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

    static affine_transform<Field, Dim> scaling(const Field &s)
    {
        return {fixmat<Field, Dim, Dim>::eye() * s, fixvec<Field, Dim>::zero()};
    }

    template<class... Args>
    static affine_transform<Field, Dim> anisotropic_scaling(Args... args)
    {
        affine_transform<Field, Dim> result = {
            fixmat<Field, Dim, Dim>::eye(),
            fixvec<Field, Dim>::zero()
        };

        std::array<Field, Dim> sf = {args...};

        for(size_t i = 0; i < Dim; ++i)
            result.linear(i,i) = sf[i];

        return result;
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

/// Compose transforms.
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

/// Apply an affine transform to a vector.
template<class Field, size_t Dim>
fixvec<Field, Dim> operator*(
    const affine_transform<Field, Dim> &a,
    const fixvec<Field, Dim> &b
)
{
    return a.linear * b + a.offset;
}

/// Extract the linear transform that applies to normal vectors.
template<class Field, size_t Dim>
auto normal_linear_map(const affine_transform<Field, Dim> &t)
{
    return transpose(inverse(t.linear));
}

/// Invert a fixed affine transform.
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

    fixmat<Field, Dim+1, Dim+1> inv_total = inverse(total);

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
