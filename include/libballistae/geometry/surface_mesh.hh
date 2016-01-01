#ifndef BALLISTAE_GEOMETRY_SURFACE_MESH_HH
#define BALLISTAE_GEOMETRY_SURFACE_MESH_HH

#include <libballistae/geometry.hh>
#include <libballistae/geometry/tri_mesh.hh>
#include <libballistae/kd_tree.hh>

namespace ballistae
{

class surface_mesh : public ballistae::geometry
{
    tri_mesh mesh;
    kd_tree<double, 3, tri_face_crunched> mesh_crushed;
    double last_crush_time = std::numeric_limits<double>::quiet_NaN();

public:
    surface_mesh(const tri_mesh &mesh_in);

    surface_mesh(const surface_mesh &other) = default;
    surface_mesh(surface_mesh &&other) = default;

    virtual ~surface_mesh();

    virtual aabox<double, 3> get_aabox();

    virtual void crush(double time);

    virtual contact<double> ray_into(
        const ray_segment<double,3> &query
    ) const;

    virtual contact<double> ray_exit(
        const ray_segment<double,3> &query
    ) const;
};

surface_mesh surface_mesh_from_obj_file(
    std::string filename,
    bool swapyz
);

}

#endif
