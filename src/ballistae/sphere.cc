#include <ballistae/sphere.hh>

#include <limits>
#include <tuple>

#include <armadillo>

#include <ballistae/contact.hh>
#include <ballistae/ray.hh>

#ifdef BALLISTAE_DEBUG
#include <iostream>
#endif

namespace ballistae
{

sphere::sphere(
    const arma::vec3 &center_in,
    const double &radius_in
)
    : center(center_in),
      radius(radius_in)
{
}

contact ray_intersect(
    const dray3 &query,
    const sphere &geom
) {
    using std::sqrt;

    auto b = arma::dot(query.slope, query.point - geom.center);
    auto c = arma::dot(query.point - geom.center, query.point - geom.center)
        - geom.radius * geom.radius;

    // We rely on std::sqrt's mandated NaN behavior.
    auto t = -b - sqrt(b * b - c);
    auto point = eval_ray(query, t);
    auto normal = arma::normalise(point - geom.center);

    return contact(
        t,
        eval_ray(query, t),
        normal
    );
}

}
