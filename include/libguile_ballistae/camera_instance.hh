#ifndef LIBGUILE_BALLISTAE_CAMERA_INSTANCE_HH
#define LIBGUILE_BALLISTAE_CAMERA_INSTANCE_HH

#include <memory>
#include <vector>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/camera_plugin_interface.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

extern scm_t_bits camera_subsmob_flags;

namespace camera_instance
{

void init(std::vector<subsmob_fns> &ss_dispatch);

SCM make(SCM plug_name, SCM config_alist);

std::shared_ptr<ballistae::camera_priv> sp_from_scm(SCM obj);

SCM ensure_type(SCM obj);

////////////////////////////////////////////////////////////////////////////////
/// Essential functions for the [camera] subsmob.
////////////////////////////////////////////////////////////////////////////////
size_t subsmob_free(SCM obj);
SCM    subsmob_mark(SCM obj);
int    subsmob_print(SCM obj, SCM port, scm_print_state *pstate);
SCM    subsmob_equalp(SCM a, SCM b);

}

}

#endif
