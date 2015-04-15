#ifndef LIBBALLISTAE_VECTOR_HH
#define LIBBALLISTAE_VECTOR_HH

/// Additional useful vector operations.

#include <cstddef>

#include <armadillo>

namespace ballistae
{

template<class Field, size_t Dim>
using fixvec = typename arma::Col<Field>:: template fixed<Dim>;

using vec3 = fixvec<double, 3>;

template<class Field, size_t NRows, size_t NCols>
using fixmat = typename arma::Mat<Field>:: template fixed<NRows, NCols>;

using mat33 = fixmat<double, 3, 3>;

template<class Field, size_t Dim>
fixvec<Field, Dim> reject(const fixvec<Field, Dim> &a, const fixvec<Field, Dim> &b)
{
    return b - arma::normalise(a) * (arma::dot(a, b) / arma::norm(a));
}

template<class Field, size_t Dim>
fixvec<Field, Dim> reflect(
    const fixvec<Field, Dim> &a,
    const fixvec<Field, Dim> &n
)
{
    using arma::dot;
    return a - Field(2) * dot(a, n) * n;
}

template<class Field>
constexpr Field epsilon()
{
    return Field(1) / Field(10000);
}

}

#endif
