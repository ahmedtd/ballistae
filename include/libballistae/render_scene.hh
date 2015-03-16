#ifndef LIBBALLISTAE_RENDER_SCENE_HH
#define LIBBALLISTAE_RENDER_SCENE_HH

#include <atomic>
#include <memory>
#include <vector>

#include <libballistae/image.hh>
#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/spectrum.hh>

namespace ballistae
{

class camera_priv;

image<float> render_scene(
    size_t img_rows,
    size_t img_cols,
    const std::shared_ptr<const camera_priv> &the_camera,
    const scene &scene,
    unsigned int supersample_factor,
    std::atomic_size_t &cur_progress
);

}

#endif
