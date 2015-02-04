#include <ballistae/cylinder.hh>

#include <armadillo>

#include <ballistae/contact.hh>
#include <ballistae/ray.hh>
#include <ballistae/vector.hh>

namespace ballistae
{

cylinder::cylinder(
    const arma::vec3 &center_in,
    const arma::vec3 &axis_in,
    const double &radius_in
)
    : center(center_in),
      axis(arma::normalise(axis_in)),
      radius(radius_in)
{
}

contact ray_intersect(
    const dray3 &query,
    const cylinder &geom
)
{
    using std::sqrt;

    arma::vec3 foil_a = query.slope - geom.axis * arma::dot(query.slope, geom.axis);
    arma::vec3 foil_b = query.point - geom.center
        - geom.axis * arma::dot(query.point - geom.center, geom.axis);

    double a = arma::dot(foil_a, foil_a);
    double b = 2.0 * arma::dot(foil_a, foil_b);
    double c = arma::dot(foil_b, foil_b) - geom.radius * geom.radius;

    double t = (-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a);

    arma::vec3 p = eval_ray(query, t);

    arma::vec3 n = arma::normalise(reject(geom.axis, p - geom.center));

    return contact(t, p, n);

}

}
