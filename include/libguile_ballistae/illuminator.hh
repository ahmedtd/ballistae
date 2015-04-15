#ifndef LIBGUILE_BALLISTAE_ILLUMINATOR_HH
#define LIBGUILE_BALLISTAE_ILLUMINATOR_HH

#include <libballistae/illuminator.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

namespace illuminator
{

subsmob_fns init();

SCM make(SCM name, SCM config_alist);

ballistae::illuminator* p_from_scm(SCM geom);

}

}

#endif
