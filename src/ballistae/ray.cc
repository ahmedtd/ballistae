#include <ballistae/ray.hh>

#include <armadillo>

namespace ballistae
{

arma::vec3 eval_ray(const dray3 &r, const double &t)
{
    return r.point + t * r.slope;
}

}
