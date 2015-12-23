#include <libballistae/geometry/surface_mesh.hh>

#include <utility>

#include <libballistae/kd_tree.hh>

#include <libballistae/geometry/load_obj.hh>
#include <libballistae/geometry/tri_mesh.hh>

using namespace frustum;

namespace ballistae
{

surface_mesh::surface_mesh(const tri_mesh &mesh_in)
    : mesh(mesh_in)
{
}

surface_mesh::~surface_mesh()
{
}

aabox<double, 3> surface_mesh::get_aabox()
{
    return mesh_crushed.root->bounds;
}

void surface_mesh::crush(const scene &the_scene, double time)
{
    if(time != last_crush_time)
        mesh_crushed = crunch(mesh);
    last_crush_time = time;
}

contact<double> surface_mesh::ray_into(
    const scene &the_scene,
    const ray_segment<double,3> &query
) const
{
    return tri_mesh_contact(query, mesh_crushed, CONTACT_INTO);
}

contact<double> surface_mesh::ray_exit(
    const scene &the_scene,
    const ray_segment<double,3> &query
) const
{
    return tri_mesh_contact(query, mesh_crushed, CONTACT_EXIT);
}

surface_mesh surface_mesh_from_obj_file(
    std::string filename,
    bool swapyz
)
{
    int errc;
    size_t err_line;
    tri_mesh the_mesh;
    std::tie(errc, err_line, the_mesh) = tri_mesh_load_obj(filename, swapyz);
    return (surface_mesh(std::move(the_mesh)));
}

}
