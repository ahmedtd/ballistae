#ifndef LIBBALLISTAE_SCENE_HH
#define LIBBALLISTAE_SCENE_HH

#include <memory>
#include <vector>

#include <frustum-0/geometry/affine_transform.hh>

#include <libballistae/contact.hh>
#include <libballistae/illuminator.hh>
#include <libballistae/kd_tree.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

class geometry;
class material;
template<size_t>
class mtlmap;

struct scene_element
{
    size_t geometry_index;
    size_t material_index;

    /// The transform that takes model space to world space
    affine_transform<double, 3> model_to_world;
};

struct crushed_scene_element
{
    geometry *the_geometry;
    material *the_material;

    /// The transform that takes a ray from world space to model space.
    affine_transform<double, 3> world_to_model;

    /// The transform that takes rays and contacts from model space to world
    /// space.
    affine_transform<double, 3> model_to_world;

    /// The linear map that takes normal vectors from model space to world
    /// space.
    fixmat<double, 3, 3> model_to_world_normals;

    /// The element's bounding box in world coordinates.
    aabox<double, 3> world_aabox;
};

struct scene
{
    /// Material maps producing 1-, 2-,and 3-vectors.
    std::vector<std::unique_ptr<mtlmap<1>>> mtlmaps_1;
    std::vector<std::unique_ptr<mtlmap<2>>> mtlmaps_2;
    std::vector<std::unique_ptr<mtlmap<3>>> mtlmaps_3;

    std::vector<std::unique_ptr<material>> materials;
    std::vector<std::unique_ptr<illuminator>> illuminators;
    std::vector<std::unique_ptr<geometry>> geometries;

    std::vector<scene_element> elements;

    kd_tree<double, 3, crushed_scene_element> crushed_elements;
};

void crush(scene &the_scene, double time);

std::tuple<contact<double>, const crushed_scene_element*>
scene_ray_intersect(
    const scene &the_scene,
    ray_segment<double, 3> query
);

}

#endif
