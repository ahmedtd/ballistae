#ifndef LIBGUILE_BALLISTAE_CAMERA_PLUGIN_INTERFACE_HH
#define LIBGUILE_BALLISTAE_CAMERA_PLUGIN_INTERFACE_HH

#include <cstddef> // workaround for bug in GMP
#include <libguile.h>

#include <libballistae/camera.hh>

extern "C"
ballistae::camera* guile_ballistae_camera(
    SCM config_alist
) __attribute__((visibility("default")));

namespace ballistae_guile
{
using create_camera_t = decltype(&guile_ballistae_camera);
}

#endif
