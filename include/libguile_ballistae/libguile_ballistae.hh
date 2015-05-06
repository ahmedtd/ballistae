#ifndef LIBBALLISTAE_GUILE_LIBBALLISTAE_GUILE_HH
#define LIBBALLISTAE_GUILE_LIBBALLISTAE_GUILE_HH

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/affine_transform.hh>
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

ballistae::affine_transform<double, 3> affine_from_scm(SCM t_scm) BG_PUBLIC;
SCM compose(SCM rest_scm) BG_PUBLIC;
SCM identity() BG_PUBLIC;
SCM translation(SCM t_scm) BG_PUBLIC;
SCM scaling(SCM s_scm) BG_PUBLIC;
SCM rotation(SCM axis_scm, SCM angle_scm) BG_PUBLIC;
SCM basis_mapping(SCM t0_scm, SCM t1_scm, SCM t2_scm) BG_PUBLIC;

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

ballistae::geometry* geometry_from_scm(SCM geom) BG_PUBLIC;
SCM geometry_make(SCM plug_soname, SCM config_alist) BG_PUBLIC;

ballistae::illuminator* illuminator_from_scm(SCM geom) BG_PUBLIC;
SCM dir_illuminator_make(SCM config_alist) BG_PUBLIC;
SCM point_isotropic_illuminator_make(SCM config_alist) BG_PUBLIC;
SCM illuminator_make(SCM name, SCM config_alist) BG_PUBLIC;

ballistae::material* material_from_scm(SCM matr) BG_PUBLIC;
SCM material_make(SCM create_fn_scm, SCM config_alist) BG_PUBLIC;
SCM material_update(SCM update_fn_scm, SCM material, SCM config) BG_PUBLIC;

ballistae::scene* scene_from_scm(SCM scene) BG_PUBLIC;
SCM scene_make() BG_PUBLIC;
SCM add_element(SCM scene, SCM geometry, SCM material, SCM transform) BG_PUBLIC;
SCM set_element_transform(SCM scene, SCM index, SCM transform) BG_PUBLIC;
SCM get_element_transform(SCM scene, SCM index) BG_PUBLIC;
SCM add_illuminator(SCM scene, SCM illuminator) BG_PUBLIC;
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
