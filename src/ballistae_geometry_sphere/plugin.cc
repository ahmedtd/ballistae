#include <libballistae/geometry.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>

#include <cmath>

#include <random>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

#include <libguile_armadillo/libguile_armadillo.hh>

namespace bl = ballistae;

class sphere_priv : public ballistae::geometry
{
public:
    sphere_priv();

    virtual ~sphere_priv();

    virtual bl::contact<double> ray_into(
        const bl::scene &the_scene,
        const bl::ray_segment<double,3> &query,
        std::ranlux24 &thread_rng
    ) const;

    virtual bl::contact<double> ray_exit(
        const bl::scene &the_scene,
        const bl::ray_segment<double,3> &query,
        std::ranlux24 &thread_rng
    ) const;
};

sphere_priv::sphere_priv()
{
}

sphere_priv::~sphere_priv()
{
}

bl::contact<double> sphere_priv::ray_into(
    const bl::scene &the_scene,
    const bl::ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    using std::acos;
    using std::atan2;
    using std::sqrt;

    auto b = arma::dot(query.the_ray.slope, query.the_ray.point);
    auto c = arma::dot(query.the_ray.point, query.the_ray.point) - 1.0;

    // We rely on std::sqrt's mandated NaN behavior.
    auto t_min = -b - sqrt(b * b - c);

    if(ballistae::contains(query.the_segment, t_min))
    {
        bl::contact<double> result;

        auto p = ballistae::eval_ray(query.the_ray, t_min);
        result.t = t_min;
        result.p = p;
        result.n = arma::normalise(p);
        result.uv = {atan2(p(0), p(1)), acos(p(2))};
        result.uvw = p;
        result.r = query.the_ray;

        return result;
    }
    else
    {
        return bl::contact<double>::nan();
    }
}

bl::contact<double> sphere_priv::ray_exit(
    const bl::scene &the_scene,
    const bl::ray_segment<double,3> &query,
    std::ranlux24 &thread_rng
) const
{
    using std::acos;
    using std::atan2;
    using std::sqrt;

    auto b = arma::dot(query.the_ray.slope, query.the_ray.point);
    auto c = arma::dot(query.the_ray.point, query.the_ray.point) - 1.0;

    // We rely on std::sqrt's mandated NaN behavior.
    auto t_max = -b + sqrt(b * b - c);

    if(ballistae::contains(query.the_segment, t_max))
    {
        bl::contact<double> result;
        auto p = ballistae::eval_ray(query.the_ray, t_max);
        result.t = t_max;
        result.p = p;
        result.n = arma::normalise(p);
        result.uv = {atan2(p(0), p(1)), acos(p(2))};
        result.uvw = p;
        result.r = query.the_ray;

        return result;
    }
    else
    {
        return bl::contact<double>::nan();
    }
}

ballistae::geometry* guile_ballistae_geometry(SCM config_alist)
{
    return new sphere_priv();
}
