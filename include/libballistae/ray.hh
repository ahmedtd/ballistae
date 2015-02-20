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
struct dray3
{
    arma::vec3 point;
    arma::vec3 slope;
};

/// Evaluate R for the given parameter value T.
///
/// Return Value:
///
///   (arma::vec3) The point along the ray with parameter value T.
arma::vec3 eval_ray(const dray3 &r, const double &t) __attribute__((pure));

}

#endif
