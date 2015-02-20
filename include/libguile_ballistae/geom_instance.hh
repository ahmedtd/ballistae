#ifndef LIBGUILE_BALLISTAE_GEOM_INSTANCE_HH
#define LIBGUILE_BALLISTAE_GEOM_INSTANCE_HH

#include <vector>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

extern scm_t_bits geom_instance_flags;

namespace geom_instance
{

void init(std::vector<subsmob_fns> &ss_dispatch);

////////////////////////////////////////////////////////////////////////////////
/// Make a geom_instance.
////////////////////////////////////////////////////////////////////////////////
SCM make(SCM plug_soname, SCM config_alist);

SCM geom_p(SCM);

////////////////////////////////////////////////////////////////////////////////
/// Assert that the argument is a geom_instance.
////////////////////////////////////////////////////////////////////////////////
SCM assert_type(SCM obj);

////////////////////////////////////////////////////////////////////////////////
/// Essential functions for the geom_instance subsmob.
////////////////////////////////////////////////////////////////////////////////
size_t subsmob_free(SCM obj);
SCM    subsmob_mark(SCM obj);
int    subsmob_print(SCM obj, SCM port, scm_print_state *pstate);
SCM    subsmob_equalp(SCM a, SCM b);

}

}

#endif
