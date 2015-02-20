#ifndef LIBBALLISTAE_CAMERA_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_CAMERA_PLUGIN_INTERFACE_HH

#include <armadillo>

#include <libballistae/ray.hh>

namespace ballistae
{

class camera_priv
{
public:
    virtual ~camera_priv() {}

    virtual dray3 image_to_ray(const arma::vec3 &image_coords) const = 0;
};

}

#endif
