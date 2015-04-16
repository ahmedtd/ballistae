#ifndef LIBBALLISTAE_RENDER_SCENE_HH
#define LIBBALLISTAE_RENDER_SCENE_HH

#include <atomic>
#include <vector>

#include <libballistae/camera.hh>
#include <libballistae/image.hh>
#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>

namespace ballistae
{

image<float> render_scene(
    size_t img_rows,
    size_t img_cols,
    const camera &the_camera,
    const scene &scene,
    unsigned int supersample_factor,
    std::atomic_size_t &cur_progress,
    const std::vector<size_t> &sampling_profile,
    const span<double> &sample_bandwidth
);

}

#endif
