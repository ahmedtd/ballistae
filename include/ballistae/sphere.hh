#ifndef BALLISTAE_SPHERE_HH
#define BALLISTAE_SPHERE_HH

#include <armadillo>

#include <ballistae/contact.hh>
#include <ballistae/ray.hh>

namespace ballistae
{

struct sphere
{
    arma::vec3 center;
    double radius;

    sphere(const arma::vec3 &center_in, const double &radius_in);
};

/// Determine the intersection of a ray with a sphere.
contact ray_intersect(
    const dray3 &query,
    const sphere &geom
);

}

#endif
