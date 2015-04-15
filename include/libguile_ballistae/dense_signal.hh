#ifndef LIBGUILE_BALLISTAE_SPECTRUM_HH
#define LIBGUILE_BALLISTAE_SPECTRUM_HH

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/dense_signal.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

namespace dense_signal
{

SCM ensure_type(SCM obj);

ballistae::dense_signal<double> from_scm(SCM obj);

subsmob_fns init();

}

}

#endif
