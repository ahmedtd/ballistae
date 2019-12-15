#ifndef LIBBALLISTAE_CAMERA_HH
#define LIBBALLISTAE_CAMERA_HH

#include <random>

#include "include/libballistae/ray.hh"
#include "include/libballistae/vector.hh"

namespace ballistae
{

class camera
{
public:
    virtual ~camera() {}

    virtual ray<double, 3> image_to_ray(
        const fixvec<double, 3> &image_coords,
        std::mt19937 &thread_rng
    ) const = 0;
};

}

#endif
