#ifndef LIBBALLISTAE_CAMERA_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_CAMERA_PLUGIN_INTERFACE_HH

#include <armadillo>

#include <libballistae/ray.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

class camera
{
public:
    virtual ~camera() {}

    virtual ray<double, 3> image_to_ray(
        const fixvec<double, 3> &image_coords
    ) const = 0;
};

}

#endif
