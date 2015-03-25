#ifndef LIBGUILE_BALLISTAE_AFFINE_TRANSFORM_HH
#define LIBGUILE_BALLISTAE_AFFINE_TRANSFORM_HH

#include <vector>

#include <libballistae/affine_transform.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

extern scm_t_bits affine_transform_subsmob_flags;

namespace affine_transform
{

namespace bl = ballistae;

void init(std::vector<subsmob_fns> &ss_dispatch);

bl::affine_transform<double, 3> scm_to_affine_transform(SCM t_scm);

}

}

#endif
