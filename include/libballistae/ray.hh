#ifndef LIBBALLISTAE_RAY_HH
#define LIBBALLISTAE_RAY_HH

#include <frustum-0/geometry/affine_transform.hh>

#include <libballistae/span.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

/// A ray, emanating from POINT in direction SLOPE.
///
/// Invariants:
///
///   * (norm(slope) == 1): SLOPE is a unit vector.
template<class Field, size_t Dim>
struct ray
{
    fixvec<double, 3> point;
    fixvec<double, 3> slope;
};

template<size_t Dim>
using dray = ray<double, Dim>;

using dray3 = ray<double, 3>;

/// Evaluate R for the given parameter value T.
///
/// Return Value:
///
///   (arma::vec3) The point along the ray with parameter value T.
template<class Field, size_t Dim>
fixvec<Field, Dim> eval_ray(const ray<Field, Dim> &r, const Field &t)
    __attribute__((visibility("default")));

template<class Field, size_t Dim>
fixvec<Field, Dim> eval_ray(const ray<Field, Dim> &r, const Field &t)
{
    return r.point + t * r.slope;
}

template<class Field, size_t Dim>
ray<Field, Dim> operator*(
    const affine_transform<Field, Dim> &a,
    const ray<Field, Dim> &b
) __attribute__((visibility("default")));

template<class Field, size_t Dim>
ray<Field, Dim> operator*(
    const affine_transform<Field, Dim> &a,
    const ray<Field, Dim> &b
)
{
    return {
        a.linear * b.point + a.offset,
        normalise(a.linear * b.slope)
    };
}

template<class Field, size_t D>
struct ray_segment
{
    ray<Field, D> the_ray;
    span<Field> the_segment;
};

template<class Field, size_t Dim>
ray_segment<Field, Dim> operator*(
    const affine_transform<Field, Dim> &a,
    const ray_segment<Field, Dim> &b
) __attribute__((visibility("default")));

template<class Field, size_t Dim>
ray_segment<Field, Dim> operator*(
    const affine_transform<Field, Dim> &a,
    const ray_segment<Field, Dim> &b
)
{
    ray_segment<Field, Dim> result;

    result.the_ray.point = a * b.the_ray.point;
    result.the_ray.slope = a.linear * b.the_ray.slope;

    // Assume that the input ray has unit slope.
    double scale_factor = norm(result.the_ray.slope);
    result.the_ray.slope /= scale_factor;

    result.the_segment = scale_factor * b.the_segment;

    return result;
}

}

#endif
