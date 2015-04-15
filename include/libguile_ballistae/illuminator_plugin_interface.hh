#ifndef LIBGUILE_BALLISTAE_ILLUMINATOR_PLUGIN_INTERFACE_HH
#define LIBGUILE_BALLISTAE_ILLUMINATOR_PLUGIN_INTERFACE_HH

#include <libballistae/illuminator.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

extern "C" ballistae::illuminator* guile_ballistae_illuminator(SCM alist)
    __attribute__((visibility("default")));

namespace ballistae_guile
{
using create_illuminator_t = decltype(&guile_ballistae_illuminator);
}

#endif
