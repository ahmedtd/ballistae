#ifndef LIBGUILE_BALLISTAE_GEOMETRY_PLUGIN_INTERFACE_HH
#define LIBGUILE_BALLISTAE_GEOMETRY_PLUGIN_INTERFACE_HH

#include <memory>

#include <cstddef> // workaround for bug in GMP
#include <libguile.h>

#include <libballistae/geom_plugin_interface.hh>

extern "C" std::shared_ptr<ballistae::geom_priv>
ballistae_geom_create_from_alist(
    SCM alist
) __attribute__((visibility("default")));

namespace ballistae_guile
{
using geomplug_create_from_alist_t = decltype(&ballistae_geom_create_from_alist);
}

#endif
