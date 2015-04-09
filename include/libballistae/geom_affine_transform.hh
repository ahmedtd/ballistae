#ifndef LIBBALLISTAE_GEOM_AFFINE_TRANSFORM_HH
#define LIBBALLISTAE_GEOM_AFFINE_TRANSFORM_HH

#include <cstddef>

#include <memory>

#include <libballistae/affine_transform.hh>
#include <libballistae/geom_plugin_interface.hh>
#include <libballistae/scene.hh>

namespace ballistae
{

class geom_affine_transform : public geom_priv
{
public:

    affine_transform<double, 3> tform;
    fixmat<double, 3, 3> invtrans;

    std::shared_ptr<geom_priv> child;

    geom_affine_transform(
        const affine_transform<double, 3> &tform_in
    );

    virtual ~geom_affine_transform();

    virtual span<double> ray_intersect(
        const scene &the_scene,
        const dray3 &query,
        const span<double> &must_overlap,
        std::mt19937 &thread_rng
    ) const;

    virtual flattened_geom flatten() const;
};

}

#endif
