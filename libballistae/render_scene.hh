#ifndef LIBBALLISTAE_RENDER_SCENE_HH
#define LIBBALLISTAE_RENDER_SCENE_HH

#include <cstdlib>
#include <functional>

#include "libballistae/camera.hh"
#include "libballistae/image.hh"
#include "libballistae/ray.hh"
#include "libballistae/scene.hh"
#include "libballistae/span.hh"

namespace ballistae {

void render_scene(const options &the_options, const camera &the_camera,
                  const scene &the_scene,
                  std::function<void(size_t, size_t)> progress_function);

}

#endif
