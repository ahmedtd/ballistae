#ifndef LIBBALLISTAE_SCENE_HH
#define LIBBALLISTAE_SCENE_HH

#include <atomic>
#include <memory>
#include <random>
#include <tuple>
#include <vector>

#include "include/frustum-0/geometry/affine_transform.hh"
#include "include/libballistae/camera.hh"
#include "include/libballistae/color.hh"
#include "include/libballistae/contact.hh"
#include "include/libballistae/geometry.hh"
#include "include/libballistae/image.hh"
#include "include/libballistae/kd_tree.hh"
#include "include/libballistae/material.hh"
#include "include/libballistae/material_map.hh"
#include "include/libballistae/ray.hh"
#include "include/libballistae/span.hh"
#include "include/libballistae/vector.hh"

namespace ballistae {

struct options {
  size_t gridsize;

  size_t img_rows;
  size_t img_cols;

  double lambda_min;
  double lambda_max;

  size_t maxdepth;

  std::string asset_dir;
  std::string output_file;
};

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
  aabox<double, 3> world_aabox;
};

struct scene {
  std::vector<scene_element> elements;

  kd_tree<double, 3, crushed_scene_element> crushed_elements;
};

void crush(scene &the_scene, double time);

std::tuple<contact<double>, const crushed_scene_element *> scene_ray_intersect(
    const scene &the_scene, ray_segment<double, 3> query);

}  // namespace ballistae

#endif
