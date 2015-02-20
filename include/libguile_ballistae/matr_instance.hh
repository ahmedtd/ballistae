#ifndef LIBGUILE_BALLISTAE_MATR_INSTANCE_HH
#define LIBGUILE_BALLISTAE_MATR_INSTANCE_HH

#include <vector>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

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
SCM make(SCM plug_name, SCM config_alist);

////////////////////////////////////////////////////////////////////////////////
/// Predicate: is obj a [matr]?
////////////////////////////////////////////////////////////////////////////////
SCM matr_p(SCM obj);

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
