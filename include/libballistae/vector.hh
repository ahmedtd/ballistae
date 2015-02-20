#ifndef LIBBALLISTAE_VECTOR_HH
#define LIBBALLISTAE_VECTOR_HH

/// Additional useful vector operations.

#include <armadillo>

namespace ballistae
{

arma::vec3 reject(const arma::vec3 &a, const arma::vec3 &b) __attribute__((pure));

}

#endif
