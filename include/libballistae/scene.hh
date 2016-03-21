#ifndef LIBBALLISTAE_SCENE_HH
#define LIBBALLISTAE_SCENE_HH

#include <atomic>
#include <memory>
#include <random>
#include <vector>

#include <frustum-0/geometry/affine_transform.hh>

#include <libballistae/camera.hh>
#include <libballistae/color.hh>
#include <libballistae/contact.hh>
#include <libballistae/geometry.hh>
#include <libballistae/image.hh>
#include <libballistae/kd_tree.hh>
#include <libballistae/material.hh>
#include <libballistae/material_map.hh>
#include <libballistae/ray.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

struct options
{
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
template<size_t>
class mtlmap;

struct scene_element
{
    geometry *the_geometry;
    material *the_material;

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
    std::vector<scene_element> elements;

    kd_tree<double, 3, crushed_scene_element> crushed_elements;
};

void crush(scene &the_scene, double time)
{
    // Crush individual geometries, materials, material maps, etc.
    //
    // After crushing, their bounding boxes must be constant (for geometries).

    // Geometry, materials, and material maps are crushed in a dependency-based
    // fashion, with each crushing its own dependencies.  To prevent redundant
    // (potentially-expensive) crushes, objects should cache the last time they
    // were crushed.

    // Produce crushed scene elements from the uncrushed scene elements.  We
    // also precompute transforms derived from each element's transform.
    std::vector<crushed_scene_element> crushed_elts;
    for(const auto &elt : the_scene.elements)
    {
        elt.the_geometry->crush(time);
        elt.the_material->crush(time);

        auto world_aabox = elt.model_to_world * elt.the_geometry->get_aabox();

        crushed_scene_element crush_elt = {
            elt.the_geometry,
            elt.the_material,
            inverse(elt.model_to_world),
            elt.model_to_world,
            normal_linear_map(elt.model_to_world),
            world_aabox
        };
        crushed_elts.push_back(crush_elt);
    }

    the_scene.crushed_elements = kd_tree<double, 3, crushed_scene_element>(
        std::move(crushed_elts),
        [](const auto &s){return s.world_aabox;}
    );

    // Refine using the surface area heuristic.
    kd_tree_refine_sah(
        the_scene.crushed_elements.root.get(),
        [](const auto &s){return s.world_aabox;},
        1.0,
        0.9
    );
}

std::tuple<contact<double>, const crushed_scene_element*>
scene_ray_intersect(
    const scene &the_scene,
    ray_segment<double, 3> query
)
{
    contact<double> min_contact;
    const crushed_scene_element *min_element = nullptr;

    auto selector = [&](const aabox<double, 3> &box) -> bool {
        using std::isnan;
        auto overlap = ray_test(query, box);
        return !isnan(overlap);
    };

    auto computor = [&](const crushed_scene_element &elt) -> void {
        using std::isnan;
        auto mdl_query = elt.world_to_model * query;
        auto entry_contact = elt.the_geometry->ray_into(mdl_query);
        if(!isnan(entry_contact.t) && contains(mdl_query.the_segment, entry_contact.t))
        {
            entry_contact = contact_transform(
                entry_contact,
                elt.model_to_world,
                elt.model_to_world_normals
            );
            query.the_segment.hi = entry_contact.t;
            min_contact = entry_contact;
            min_element = &elt;

            // Remake mdl_query from the updated world-space query.
            mdl_query = elt.world_to_model * query;
        }
        auto exit_contact = elt.the_geometry->ray_exit(mdl_query);
        if(!isnan(exit_contact.t) && contains(mdl_query.the_segment, exit_contact.t))
        {
            exit_contact = contact_transform(
                exit_contact,
                elt.model_to_world,
                elt.model_to_world_normals
            );
            query.the_segment.hi = exit_contact.t;
            min_contact = exit_contact;
            min_element = &elt;
        }
    };

    the_scene.crushed_elements.query(selector, computor);

    return std::make_tuple(min_contact, min_element);
}

}

#endif
