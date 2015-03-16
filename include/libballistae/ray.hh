#ifndef LIBBALLISTAE_RAY_HH
#define LIBBALLISTAE_RAY_HH

#include <armadillo>

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
    using vec_type = typename arma::Col<Field>::template fixed<Dim>;
    vec_type point;
    vec_type slope;
};

template<size_t Dim>
using dray = ray<double, Dim>;

using dray3 = ray<double, 3>;

/// Evaluate R for the given parameter value T.
///
/// Return Value:
///
///   (arma::vec3) The point along the ray with parameter value T.
arma::vec3 eval_ray(const dray3 &r, const double &t) __attribute__((pure));

}

#endif
