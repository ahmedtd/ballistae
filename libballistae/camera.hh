#ifndef LIBBALLISTAE_CAMERA_HH
#define LIBBALLISTAE_CAMERA_HH

#include <random>

#include "libballistae/ray.hh"
#include "libballistae/vector.hh"

namespace ballistae {

class camera {
 public:
  virtual ~camera() {}

  virtual ray image_to_ray(std::size_t cur_row, std::size_t img_rows,
                           std::size_t cur_col, std::size_t img_cols,
                           std::mt19937 &rng) const = 0;
};

}  // namespace ballistae

#endif
