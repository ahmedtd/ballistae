#ifndef LIBGUILE_BALLISTAE_GEOMETRY_PLUGIN_INTERFACE_HH
#define LIBGUILE_BALLISTAE_GEOMETRY_PLUGIN_INTERFACE_HH

#include <memory>

#include <cstddef> // workaround for bug in GMP
#include <libguile.h>

#include <libballistae/geometry.hh>
#include <libballistae/scene.hh>

extern "C"
std::unique_ptr<ballistae::geometry>
guile_ballistae_geometry(
    ballistae::scene *p_scene,
    SCM alist
) __attribute__((visibility("default")));

namespace ballistae_guile
{
using create_geometry_t = decltype(&guile_ballistae_geometry);
}

#endif
