#ifndef LIBGUILE_BALLISTAE_AFFINE_TRANSFORM_HH
#define LIBGUILE_BALLISTAE_AFFINE_TRANSFORM_HH

#include <vector>

#include <libballistae/affine_transform.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

namespace affine_transform
{

subsmob_fns init();

SCM ensure_type(SCM trans);

ballistae::affine_transform<double, 3> from_scm(SCM t_scm);

}

}

#endif
