#ifndef LIBGUILE_BALLISTAE_CAMERA_INSTANCE_HH
#define LIBGUILE_BALLISTAE_CAMERA_INSTANCE_HH

#include <memory>
#include <vector>

#include <libballistae/camera.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

namespace camera
{

subsmob_fns init();

SCM make(SCM plug_name, SCM config_alist);

ballistae::camera* p_from_scm(SCM obj);

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
