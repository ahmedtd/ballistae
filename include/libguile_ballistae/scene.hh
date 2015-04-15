#ifndef LIBGUILE_BALLISTAE_SCENE_HH
#define LIBGUILE_BALLISTAE_SCENE_HH

#include <cstddef> // workaround for bug in GMP
#include <libguile.h>

#include <libballistae/scene.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

extern scm_t_bits scene_subsmob_flags;

namespace scene
{

subsmob_fns init();

ballistae::scene* p_from_scene(SCM scene);

size_t subsmob_free(SCM obj);
SCM    subsmob_mark(SCM obj);
int    subsmob_print(SCM obj, SCM port, scm_print_state *pstate);
SCM    subsmob_equalp(SCM a, SCM b);

}

}

#endif
