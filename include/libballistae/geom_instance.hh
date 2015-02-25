#ifndef LIBBALLISTAE_GEOM_INSTANCE_HH
#define LIBBALLISTAE_GEOM_INSTANCE_HH

#include <memory>

#include <libballistae/contact.hh>
#include <libballistae/geom_plugin_interface.hh>
#include <libballistae/ray.hh>
#include <libballistae/span.hh>

namespace ballistae
{

/// A piece of geometry.
///
///
struct geom_instance final
{
    std::shared_ptr<geom_priv> data;
    geomplug_ray_intersect_t ray_intersect_fn;

    geom_instance() = delete;

    geom_instance(
        std::shared_ptr<geom_priv> &data_in,
        geomplug_ray_intersect_t ray_intersect_fn_in
    );
    
    geom_instance(geom_instance &copy_from) = default;
    geom_instance(geom_instance &&move_from) = default;

    geom_instance& operator=(geom_instance &copy_from) = default;
    geom_instance& operator=(geom_instance &&move_from) = default;
};

/// 
contact ray_intersect(
    const dray3 &query,
    const geom_instance &g,
    const span<double> &must_overlap,
    const std::size_t index
);

}

#endif
