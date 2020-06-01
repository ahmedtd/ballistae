#ifndef BALLISTAE_GEOMETRY_SURFACE_MESH_HH
#define BALLISTAE_GEOMETRY_SURFACE_MESH_HH

#include "libballistae/geometry.hh"
#include "libballistae/geometry/load_obj.hh"
#include "libballistae/geometry/tri_mesh.hh"
#include "libballistae/kd_tree.hh"

namespace ballistae {

class surface_mesh : public ballistae::geometry {
  tri_mesh mesh;
  kd_tree<tri_face_crunched> mesh_crushed;
  double last_crush_time = std::numeric_limits<double>::quiet_NaN();

 public:
  surface_mesh(const tri_mesh &mesh_in) : mesh(mesh_in) {}

  surface_mesh(const surface_mesh &other) = default;
  surface_mesh(surface_mesh &&other) = default;

  virtual ~surface_mesh() {}

  virtual aabox get_aabox() { return mesh_crushed.root->bounds; }

  virtual void crush(double time) {
    if (time != last_crush_time) {
      mesh_crushed = crunch(mesh);
    }
    last_crush_time = time;
  }

  virtual contact ray_into(const ray_segment &query) const {
    return tri_mesh_contact(query, mesh_crushed, CONTACT_INTO);
  }

  virtual contact ray_exit(const ray_segment &query) const {
    return tri_mesh_contact(query, mesh_crushed, CONTACT_EXIT);
  }
};

surface_mesh surface_mesh_from_obj_file(std::string filename, bool swapyz) {
  int errc;
  size_t err_line;
  tri_mesh the_mesh;
  std::tie(errc, err_line, the_mesh) = tri_mesh_load_obj(filename, swapyz);

  if (errc == OBJ_ERRC_FILE_NOT_OPENABLE) {
    throw std::runtime_error("could not read file: " + filename);
  } else if (errc == OBJ_ERRC_FILE_NOT_LOADABLE) {
    throw std::runtime_error("could not load file: " + filename);
  } else if (errc == OBJ_ERRC_PARSE_ERROR) {
    // TODO(ahmedtd): Do this with absl instead
    throw std::runtime_error("error at line %d while parsing file: " +
                             filename);
  } else if (errc == OBJ_ERRC_INSANE) {
    throw std::runtime_error("obj file is not self consistent: " + filename);
  } else if (errc != OBJ_ERRC_NONE) {
    throw std::runtime_error("unknown error parsing file: " + filename);
  }

  return (surface_mesh(std::move(the_mesh)));
}

}  // namespace ballistae

#endif
