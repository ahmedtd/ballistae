#include <ballistae/vector.hh>

#include <armadillo>

namespace ballistae
{

arma::vec3 reject(const arma::vec3 &a, const arma::vec3 &b)
{
    return b - arma::normalise(a) * (arma::dot(a, b) / arma::norm(a));
}

}
