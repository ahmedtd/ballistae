#ifndef LIBBALLISTAE_SCENE_HH
#define LIBBALLISTAE_SCENE_HH

#include <atomic>
#include <memory>
#include <random>
#include <tuple>
#include <vector>

#include "frustum/geometry/affine_transform.hh"
#include "libballistae/camera.hh"
#include "libballistae/color.hh"
#include "libballistae/contact.hh"
#include "libballistae/geometry.hh"
#include "libballistae/kd_tree.hh"
#include "libballistae/material.hh"
#include "libballistae/material_map.hh"
#include "libballistae/ray.hh"
#include "libballistae/span.hh"
#include "libballistae/vector.hh"

namespace ballistae {

class geometry;
class material;
template <size_t>
class mtlmap;

struct scene_element {
  geometry *the_geometry;
  material *the_material;

  /// The transform that takes model space to world space
  affine_transform<double, 3> model_to_world;
};

struct crushed_scene_element {
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
  aabox world_aabox;
};

struct scene {
  std::vector<scene_element> elements;

  kd_tree<crushed_scene_element> crushed_elements;
};

void crush(scene &the_scene, double time);

std::tuple<contact, const crushed_scene_element *> scene_ray_intersect(
    const scene &the_scene, ray_segment query);

}  // namespace ballistae

#endif
