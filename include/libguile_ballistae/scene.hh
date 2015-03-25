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

void init(std::vector<subsmob_fns> &ss_dispatch);

SCM crush(SCM geometry_material_alist);

SCM scene_p(SCM obj);

////////////////////////////////////////////////////////////////////////////////
/// Render a scene.
////////////////////////////////////////////////////////////////////////////////
SCM render_scene(
    SCM camera_scm,
    SCM scene_scm,
    SCM output_file_scm,
    SCM img_rows_scm,
    SCM img_cols_scm,
    SCM ss_factor_scm,
    SCM sample_profile_scm,
    SCM lambda_nm_profile_scm
);

size_t subsmob_free(SCM obj);
SCM    subsmob_mark(SCM obj);
int    subsmob_print(SCM obj, SCM port, scm_print_state *pstate);
SCM    subsmob_equalp(SCM a, SCM b);

}

}

#endif
