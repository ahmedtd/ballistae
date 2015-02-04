#ifndef BALLISTAE_PLANE_HH
#define BALLISTAE_PLANE_HH

#include <armadillo>

#include <ballistae/contact.hh>
#include <ballistae/ray.hh>

namespace ballistae
{

struct plane
{
    arma::vec3 center;
    arma::vec3 normal;

    plane(const arma::vec3 &center_in, const arma::vec3 &normal_in);
};

contact ray_intersect(
    const dray3 &query,
    const plane &geom
);

}

#endif
