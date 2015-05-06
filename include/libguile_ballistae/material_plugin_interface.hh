#ifndef LIBGUILE_BALLISTAE_MATR_PLUGIN_INTERFACE_HH
#define LIBGUILE_BALLISTAE_MATR_PLUGIN_INTERFACE_HH

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/material.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

////////////////////////////////////////////////////////////////////////////////
/// Initialize a material instance from the values in [alist].
////////////////////////////////////////////////////////////////////////////////
extern "C" ballistae::material* guile_ballistae_material(
    SCM alist
) __attribute__((visibility("default")));

extern "C" ballistae::material* guile_ballistae_update_material(
    ballistae::material *p_matr,
    SCM config
) __attribute__((visibility("default")));

namespace ballistae_guile
{
using create_material_t = decltype(&guile_ballistae_material);
using update_material_t = decltype(&guile_ballistae_update_material);
}

#endif
