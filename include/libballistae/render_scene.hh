#ifndef LIBBALLISTAE_RENDER_SCENE_HH
#define LIBBALLISTAE_RENDER_SCENE_HH

#include <cstdlib>

#include <atomic>
#include <vector>

#include <libballistae/camera.hh>
#include <libballistae/image.hh>
#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>

namespace ballistae
{

struct render_opts
{
    size_t gridsize;
    span<double> bandwidth;
    size_t n_lambdas;
    size_t depth_lim;
};

image<float> render_scene(
    size_t img_rows,
    size_t img_cols,
    const camera &the_camera,
    const scene &scene,
    const render_opts &opts,
    std::atomic_size_t &cur_progress
);

}

#endif
