#include <ballistae/plane.hh>

#include <armadillo>

#include <ballistae/contact.hh>
#include <ballistae/ray.hh>

namespace ballistae
{

plane::plane(
    const arma::vec3 &center_in,
    const arma::vec3 &normal_in
)
    : center(center_in),
      normal(arma::normalise(normal_in))
{
}

contact ray_intersect(
    const dray3 &query,
    const plane &geom
) {
    double t = arma::dot(geom.center - query.point, geom.normal)
        / arma::dot(query.slope, geom.normal);

    return contact(
        t,
        eval_ray(query, t),
        geom.normal
    );
}

}
