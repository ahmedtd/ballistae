#ifndef LIBBALLISTAE_SCENE_HH
#define LIBBALLISTAE_SCENE_HH

#include <memory>
#include <vector>

#define cimg_display 0
#define cimg_verbosity 1
#include <CImg.h>

#include <libballistae/camera_plugin_interface.hh>
#include <libballistae/contact.hh>
#include <libballistae/geom_plugin_interface.hh>
#include <libballistae/matr_plugin_interface.hh>
#include <libballistae/ray.hh>
#include <libballistae/spectrum.hh>

namespace ballistae
{

namespace cimg = cimg_library;

struct scene
{
    std::shared_ptr<matr_priv> infty_matr;
    std::vector<std::shared_ptr<matr_priv>> materials;
    std::vector<std::shared_ptr<geom_priv>> geometries;
};

color_d_rgb ray_intersect(const dray3 &query, const scene &the_scene);

cimg::CImg<float>& render_scene(
    cimg::CImg<float> &img_in,
    const std::shared_ptr<const camera_priv> &the_camera,
    const scene &scene,
    unsigned int supersample_factor
);

}

#endif
