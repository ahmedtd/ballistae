#ifndef BALLISTAE_GEOM_PLUGIN_SURFACE_MESH_LOAD_OBJ_HH
#define BALLISTAE_GEOM_PLUGIN_SURFACE_MESH_LOAD_OBJ_HH

#include <string>
#include <tuple>

#include "tri_mesh.hh"

constexpr int OBJ_ERRC_NONE = 0;
constexpr int OBJ_ERRC_FILE_NOT_LOADABLE = 1;
constexpr int OBJ_ERRC_PARSE_ERROR = 2;
constexpr int OBJ_ERRC_INSANE = 3;

std::tuple<int, size_t, tri_mesh> tri_mesh_load_obj(const char *const filename);

#endif
