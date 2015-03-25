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
    virtual bl::span<double> ray_intersect(
        const bl::scene &the_scene,
        const bl::dray3 &query,
        const bl::span<double> &must_overlap,
        std::mt19937 &thread_rng
    ) const;
};

bl::span<double> infty_priv::ray_intersect(
        const bl::scene &the_scene,
        const bl::dray3 &query,
        const bl::span<double> &must_overlap,
        std::mt19937 &thread_rng
) const
{
    auto infty = std::numeric_limits<double>::infinity();

    if(must_overlap.lo == -infty)
    {
        bl::span<double> result = {
            -infty,
            -infty,
            query.slope,
            -query.slope
        };

        return result;
    }
    else if(must_overlap.hi == infty)
    {
        bl::span<double> result = {
            infty,
            infty,
            -query.slope,
            query.slope
        };
        return result;
    }
    else
    {
        return bl::span<double>::nan();
    }
}

std::shared_ptr<ballistae::geom_priv> ballistae_geom_create_from_alist(
    SCM config_alist
)
{
    return std::make_shared<infty_priv>();
}
