#ifndef LIBGUILE_BALLISTAE_MATR_PLUGIN_INTERFACE_HH
#define LIBGUILE_BALLISTAE_MATR_PLUGIN_INTERFACE_HH

#include <memory>

#include <cstddef> // workaround for bug in GMP
#include <libguile.h>

#include <libballistae/matr_plugin_interface.hh>

////////////////////////////////////////////////////////////////////////////////
/// Initialize a material instance from the values in [alist].
////////////////////////////////////////////////////////////////////////////////
extern "C" std::shared_ptr<ballistae::matr_priv>
ballistae_matr_create_from_alist(
    SCM alist
)__attribute__((visibility("default")));

namespace ballistae_guile
{
using matrplug_create_from_alist_t = decltype(&ballistae_matr_create_from_alist);
}

#endif
