#include <libballistae/geom_plugin_interface.hh>
#include <libguile_ballistae/geom_plugin_interface.hh>

#include <cmath>

#include <limits>
#include <memory>
#include <random>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

#include <libguile_armadillo/libguile_armadillo.hh>

namespace bl = ballistae;

class infty_priv final : public ballistae::geom_priv
{
public:
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

bl::contact<double> infty_priv::ray_into(
        const bl::scene &the_scene,
        const bl::dray3 &query,
        const bl::span<double> &must_overlap,
        std::ranlux24 &thread_rng
) const
{
    using std::acos;
    using std::atan2;

    auto infty = std::numeric_limits<double>::infinity();

    if(must_overlap.hi == infty)
    {
        bl::fixvec<double, 3> p = bl::eval_ray(query, infty);
        bl::contact<double> result = {
            infty,
            query,
            p,
            -query.slope,
            {atan2(query.slope(0), query.slope(1)), acos(query.slope(2))},
            p
        };
        return result;
    }
    else
    {
        return bl::contact<double>::nan();
    }
}

bl::contact<double> infty_priv::ray_exit(
        const bl::scene &the_scene,
        const bl::dray3 &query,
        const bl::span<double> &must_overlap,
        std::ranlux24 &thread_rng
) const
{
    auto infty = std::numeric_limits<double>::infinity();

    if(must_overlap.lo == -infty)
    {
        bl::fixvec<double, 3> p = bl::eval_ray(query, infty);
        bl::contact<double> result = {
            -infty,
            query,
            p,
            query.slope,
            {atan2(-query.slope(0), -query.slope(1)), acos(-query.slope(2))},
            p
        };
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
    return std::make_shared<infty_priv>();
}
