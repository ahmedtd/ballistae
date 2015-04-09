#include <libballistae/geom_plugin_interface.hh>
#include <libguile_ballistae/geom_plugin_interface.hh>

#include <cmath>

#include <memory>
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

class sphere_priv : public ballistae::geom_priv
{
public:
    sphere_priv();

    virtual ~sphere_priv();

    virtual bl::contact<double> ray_into(
        const bl::scene &the_scene,
        const bl::dray3 &query,
        const bl::span<double> &must_overlap,
        std::ranlux24 &thread_rng
    ) const;

    virtual bl::contact<double> ray_exit(
        const bl::scene &the_scene,
        const bl::dray3 &query,
        const bl::span<double> &must_overlap,
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
    const bl::dray3 &query,
    const bl::span<double> &must_overlap,
    std::ranlux24 &thread_rng
) const
{
    using std::acos;
    using std::atan2;
    using std::sqrt;

    auto b = arma::dot(query.slope, query.point);
    auto c = arma::dot(query.point, query.point) - 1.0;

    // We rely on std::sqrt's mandated NaN behavior.
    auto t_min = -b - sqrt(b * b - c);

    if(ballistae::contains(must_overlap, t_min))
    {
        bl::contact<double> result;

        auto p = ballistae::eval_ray(query, t_min);
        result.t = t_min;
        result.p = p;
        result.n = arma::normalise(p);
        result.uv = {atan2(p(0), p(1)), acos(p(2))};
        result.uvw = p;
        result.r = query;

        return result;
    }
    else
    {
        return bl::contact<double>::nan();
    }
}

bl::contact<double> sphere_priv::ray_exit(
    const bl::scene &the_scene,
    const bl::dray3 &query,
    const bl::span<double> &must_overlap,
    std::ranlux24 &thread_rng
) const
{
    using std::acos;
    using std::atan2;
    using std::sqrt;

    auto b = arma::dot(query.slope, query.point);
    auto c = arma::dot(query.point, query.point) - 1.0;

    // We rely on std::sqrt's mandated NaN behavior.
    auto t_max = -b + sqrt(b * b - c);

    if(ballistae::contains(must_overlap, t_max))
    {
        bl::contact<double> result;
        auto p = ballistae::eval_ray(query, t_max);
        result.t = t_max;
        result.p = p;
        result.n = arma::normalise(p);
        result.uv = {atan2(p(0), p(1)), acos(p(2))};
        result.uvw = p;
        result.r = query;

        return result;
    }
    else
    {
        return bl::contact<double>::nan();
    }
}

std::shared_ptr<ballistae::geom_priv> ballistae_geom_create_from_alist(
    SCM config_alist
)
{
    return std::make_shared<sphere_priv>();
}
