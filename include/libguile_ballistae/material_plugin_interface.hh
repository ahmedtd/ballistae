#ifndef LIBGUILE_BALLISTAE_MATR_PLUGIN_INTERFACE_HH
#define LIBGUILE_BALLISTAE_MATR_PLUGIN_INTERFACE_HH

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/material.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

struct __attribute__((visibility("default"))) updatable_material
    : public ballistae::material
{
    virtual ~updatable_material()
        __attribute__((visibility("default")));

    virtual void guile_update(ballistae::scene *scene_p, SCM config)
        __attribute__((visibility("default"))) = 0;
};

}

////////////////////////////////////////////////////////////////////////////////
/// Initialize a material instance from the values in [alist].
////////////////////////////////////////////////////////////////////////////////
extern "C" ballistae_guile::updatable_material* guile_ballistae_material(
    ballistae::scene *p_scene,
    SCM config_alist
) __attribute__((visibility("default")));

namespace ballistae_guile
{
using create_material_t = decltype(&guile_ballistae_material);
}

#endif
