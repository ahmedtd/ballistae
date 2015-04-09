#ifndef LIBGUILE_BALLISTAE_MATR_INSTANCE_HH
#define LIBGUILE_BALLISTAE_MATR_INSTANCE_HH

#include <memory>
#include <vector>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/matr_plugin_interface.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

extern scm_t_bits matr_subsmob_flags;

namespace matr_instance
{

void init(std::vector<subsmob_fns> &ss_dispatch);

////////////////////////////////////////////////////////////////////////////////
/// Make instance of [plug_name], configured using [config_alist].
////////////////////////////////////////////////////////////////////////////////
SCM make_backend(SCM plug_name, SCM config_alist);

SCM ensure_type(SCM matr);

std::shared_ptr<ballistae::matr_priv> sp_from_scm(SCM matr);

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
