#ifndef BALLISTAE_CYLINDER_HH
#define BALLISTAE_CYLINDER_HH

#include <armadillo>

#include <ballistae/contact.hh>
#include <ballistae/ray.hh>

namespace ballistae
{

struct cylinder
{
    arma::vec3 center;
    arma::vec3 axis;
    double radius;

    cylinder(
        const arma::vec3 &center_in,
        const arma::vec3 &axis_in,
        const double &radius
    );
};

contact ray_intersect(
    const dray3 &query,
    const cylinder &geom
);

}

#endif
