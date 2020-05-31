#pragma once

#include <cstdlib>
#include <functional>

#include "libballistae/camera.hh"
#include "libballistae/ray.hh"
#include "libballistae/scene.hh"
#include "libballistae/span.hh"
#include "libballistae/spectral_image.hh"

namespace ballistae {

struct options {
  size_t maxdepth;
  size_t target_subsamples;
};

void render_scene(const options &the_options, spectral_image *sample_db,
                  const camera &the_camera, const scene &the_scene,
                  std::function<void(size_t, size_t)> progress_function);
}
