#ifndef LIBBALLISTAE_GUILE_LIBBALLISTAE_GUILE_HH
#define LIBBALLISTAE_GUILE_LIBBALLISTAE_GUILE_HH

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/camera.hh>
#include <libballistae/dense_signal.hh>
#include <libballistae/geometry.hh>
#include <libballistae/illuminator.hh>
#include <libballistae/material.hh>
#include <libballistae/scene.hh>
#include <libballistae/render_scene.hh>

#define BG_PUBLIC __attribute__((visibility("default")));

////////////////////////////////////////////////////////////////////////////////
/// Entry point for the library.
////////////////////////////////////////////////////////////////////////////////
///
/// The library has a corresponding scheme file that should be used for loading.
extern "C" void libguile_ballistae_init() BG_PUBLIC;

namespace ballistae_guile
{

ballistae::camera* camera_from_scm(SCM obj) BG_PUBLIC;
SCM camera_make(SCM plug_name, SCM config_alist) BG_PUBLIC;

ballistae::dense_signal<double> signal_from_scm(SCM obj) BG_PUBLIC;
SCM signal_from_list(SCM lo, SCM hi, SCM val_list) BG_PUBLIC;
SCM signal_from_function(SCM lo_scm, SCM hi_scm, SCM n_scm, SCM fn) BG_PUBLIC;
SCM pulse(SCM pulse_src, SCM pulse_lim, SCM pulse_power) BG_PUBLIC;
SCM red(SCM intensity) BG_PUBLIC;
SCM green(SCM intensity) BG_PUBLIC;
SCM blue(SCM intensity) BG_PUBLIC;
SCM sunlight(SCM intensity) BG_PUBLIC;
SCM cie_d65() BG_PUBLIC;
SCM rgb_to_spectral(SCM red, SCM green, SCM blue) BG_PUBLIC;

ballistae::scene* scene_from_scm(SCM scene) BG_PUBLIC;
SCM scene_make() BG_PUBLIC;
SCM add_element(SCM scene, SCM geometry, SCM material, SCM transform) BG_PUBLIC;
SCM set_element_transform(SCM scene, SCM index, SCM transform) BG_PUBLIC;
SCM render(
    SCM scene,
    SCM camera,
    SCM output_file_scm,
    SCM img_rows_scm,
    SCM img_cols_scm,
    SCM opts_scm
) BG_PUBLIC;

}

#undef BG_PUBLIC

#endif
