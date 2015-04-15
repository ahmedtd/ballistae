#ifndef LIBGUILE_BALLISTAE_MATR_INSTANCE_HH
#define LIBGUILE_BALLISTAE_MATR_INSTANCE_HH

#include <memory>
#include <vector>

#include <libballistae/material.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

namespace material
{

subsmob_fns init();

////////////////////////////////////////////////////////////////////////////////
/// Make instance of [plug_name], configured using [config_alist].
////////////////////////////////////////////////////////////////////////////////
SCM make_backend(SCM plug_name, SCM config_alist);

SCM ensure_type(SCM matr);

ballistae::material* p_from_scm(SCM matr);

////////////////////////////////////////////////////////////////////////////////
/// Essential functions for the [matr] subsmob.
////////////////////////////////////////////////////////////////////////////////
size_t subsmob_free(SCM obj);
SCM    subsmob_mark(SCM obj);
int    subsmob_print(SCM obj, SCM port, scm_print_state *pstate);
SCM    subsmob_equalp(SCM a, SCM b);

}

}

#endif
