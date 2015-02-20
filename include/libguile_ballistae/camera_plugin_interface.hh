#ifndef LIBGUILE_BALLISTAE_CAMERA_PLUGIN_INTERFACE_HH
#define LIBGUILE_BALLISTAE_CAMERA_PLUGIN_INTERFACE_HH

#include <memory>

#include <cstddef> // workaround for bug in GMP
#include <libguile.h>

extern "C"
std::shared_ptr<ballistae::camera_priv> ballistae_camera_create_from_alist(
    SCM config_alist
) __attribute__((visibility("default")));

namespace ballistae_guile
{
using camera_create_from_alist_t = decltype(&ballistae_camera_create_from_alist);
}

#endif
