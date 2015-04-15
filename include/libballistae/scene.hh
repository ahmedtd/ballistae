#ifndef LIBBALLISTAE_SCENE_HH
#define LIBBALLISTAE_SCENE_HH

#include <memory>
#include <vector>

#include <libballistae/affine_transform.hh>
#include <libballistae/contact.hh>
#include <libballistae/illuminator.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

class geometry;
class material;

struct scene_element
{
    geometry *the_geometry;
    material *the_material;
    affine_transform<double, 3> forward_transform;
    affine_transform<double, 3> reverse_transform;
};

struct scene
{
    std::vector<std::unique_ptr<material>> materials;
    std::vector<std::unique_ptr<illuminator>> illuminators;
    std::vector<std::unique_ptr<geometry>> geometries;

    std::vector<scene_element> elements;
};

std::tuple<contact<double>, size_t> scene_ray_intersect(
    const scene &the_scene,
    const ray_segment<double, 3> &query,
    std::ranlux24 &thread_rng
);

}

#endif
