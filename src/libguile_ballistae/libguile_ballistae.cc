#include <libguile_ballistae/libguile_ballistae.hh>

#include <cstdlib>

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <tuple>
#include <utility>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <frustum-0/indicial/fixed.hh>
#include <libguile_frustum0/libguile_frustum0.hh>

#include <libballistae/color.hh>
#include <libballistae/dense_signal.hh>
#include <libballistae/material_map.hh>

#include <libguile_ballistae/camera_plugin_interface.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>
#include <libguile_ballistae/illuminator_plugin_interface.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

static scm_t_bits ballistae_guile_smob_tag;

namespace ballistae_guile
{

using namespace guile_frustum;

////////////////////////////////////////////////////////////////////////////////
/// Create a ballistae smob with given flags and data.
////////////////////////////////////////////////////////////////////////////////
template<class Data>
SCM new_smob(scm_t_bits flag, Data data)
{
    SCM result = scm_new_smob(ballistae_guile_smob_tag, reinterpret_cast<scm_t_bits>(data));
    SCM_SET_SMOB_FLAGS(result, flag);
    return result;
}

SCM ensure_smob(SCM obj, scm_t_bits flag);

template<typename T>
T smob_get_data(SCM obj)
{
    static_assert(
        sizeof(T) == sizeof(scm_t_bits),
        "Type to retrieve must be the same size as scm_t_bits."
    );

    return reinterpret_cast<T>(SCM_SMOB_DATA(obj));
}

template<typename T>
SCM smob_set_data(SCM obj, T data)
{
    static_assert(
        sizeof(T) == sizeof(scm_t_bits),
        "Type to store must be the same size as scm_t_bits."
    );

    return SCM_SET_SMOB_DATA(obj, reinterpret_cast<scm_t_bits>(data));
}

////////////////////////////////////////////////////////////////////////////////
/// Subsmob tags.
////////////////////////////////////////////////////////////////////////////////
///
/// Values used in the 16-bit flag field.
constexpr scm_t_bits flag_scene            = 0;
constexpr scm_t_bits flag_camera           = 1;
constexpr scm_t_bits flag_dense_signal     = 2;

/// The MSB of the flags field is used to track whether or not the subsmob has
/// been registered with a scene.  If it is, then the scene controls the
/// lifetime of the allocated object, not the smob.

constexpr scm_t_bits mask_subsmob_type = 0x7fff;
constexpr scm_t_bits mask_registered = 0x8000;

scm_t_bits get_subsmob_type(SCM obj)
{
    return SCM_SMOB_FLAGS(obj) & mask_subsmob_type;
}

SCM ensure_smob(SCM obj, scm_t_bits flag)
{
    // FOR SOME REASON THIS CALL FAILS
    scm_assert_smob_type(ballistae_guile_smob_tag, obj);
    if(get_subsmob_type(obj) != flag)
        scm_wrong_type_arg(nullptr, SCM_ARG1, obj);
    return obj;
}

////////////////////////////////////////////////////////////////////////////////
/// Camera subsmob.
////////////////////////////////////////////////////////////////////////////////

SCM camera_make(SCM plug_name, SCM config_alist)
{
    SCM plug_soname = scm_string_append(
        scm_list_2(
            scm_from_utf8_string("ballistae_camera_"),
            plug_name
        )
    );

    SCM so_handle = scm_dynamic_link(plug_soname);

    auto create_fn = reinterpret_cast<create_camera_t>(
        scm_to_pointer(
            scm_dynamic_pointer(
                scm_from_utf8_string("guile_ballistae_camera"),
                so_handle
            )
        )
    );

    return new_smob(flag_camera, create_fn(config_alist));
}

ballistae::camera* camera_from_scm(SCM obj)
{
    ensure_smob(obj, flag_camera);
    return smob_get_data<ballistae::camera*>(obj);
}

size_t camera_free(SCM obj)
{
    delete camera_from_scm(obj);
    return 0;
}

int camera_print(SCM obj, SCM port, scm_print_state *pstate)
{
    scm_write_line(scm_from_utf8_string("#<bsta/camera>"), SCM_UNDEFINED);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Dense signal subsmob.
////////////////////////////////////////////////////////////////////////////////

SCM signal_from_list(SCM lo, SCM hi, SCM val_list)
{
    auto p = new ballistae::dense_signal<double>();
    p->src_x = scm_to_double(lo);
    p->lim_x = scm_to_double(hi);

    for(; !scm_is_null(val_list); val_list = scm_cdr(val_list))
    {
        p->samples.push_back(scm_to_double(scm_car(val_list)));
    }

    return new_smob(flag_dense_signal, p);
}

SCM signal_from_function(SCM lo_scm, SCM hi_scm, SCM n_scm, SCM fn)
{
    double lo = scm_to_double(lo_scm);
    double hi = scm_to_double(hi_scm);

    auto p = new ballistae::dense_signal<double>();
    p->src_x = lo;
    p->lim_x = hi;

    size_t n = scm_to_size_t(n_scm);
    for(size_t i = 0; i < n; ++i)
    {
        double lambda = lo + i * (hi - lo) / n;
        SCM val_scm = scm_call_1(fn, scm_from_double(lambda));
        double val = scm_to_double(val_scm);
        p->samples.push_back(val);
    }

    return new_smob(flag_dense_signal, p);
}

SCM pulse(SCM pulse_src, SCM pulse_lim, SCM pulse_power)
{
    auto p = new ballistae::dense_signal<double>(
        ballistae::pulse<double>(
            390,
            835,
            89,
            scm_to_double(pulse_src),
            scm_to_double(pulse_lim),
            scm_to_double(pulse_power)
        )
    );

    return new_smob(flag_dense_signal, p);
}

SCM red(SCM intensity)
{
    auto p = new ballistae::dense_signal<double>(scm_to_double(intensity) * ballistae::red<double>());
    return new_smob(flag_dense_signal, p);
}

SCM green(SCM intensity)
{
    auto p = new ballistae::dense_signal<double>(scm_to_double(intensity) * ballistae::green<double>());
    return new_smob(flag_dense_signal, p);
}

SCM blue(SCM intensity)
{
    auto p = new ballistae::dense_signal<double>(scm_to_double(intensity) * ballistae::blue<double>());
    return new_smob(flag_dense_signal, p);
}

SCM sunlight(SCM intensity)
{
    auto p = new ballistae::dense_signal<double>(scm_to_double(intensity) * ballistae::sunlight<double>());
    return new_smob(flag_dense_signal, p);
}

SCM cie_a()
{
    auto p = new ballistae::dense_signal<double>(ballistae::cie_a<double>());
    return new_smob(flag_dense_signal, p);
}

SCM cie_d65()
{
    auto p = new ballistae::dense_signal<double>(ballistae::cie_d65<double>());
    return new_smob(flag_dense_signal, p);
}

SCM rgb_to_spectral(SCM red, SCM green, SCM blue)
{
    ballistae::color3<double, ballistae::rgb_tag> color = {
        scm_to_double(red),
        scm_to_double(green),
        scm_to_double(blue)
    };

    auto sig = ballistae::rgb_to_spectral(color);
    return new_smob(flag_dense_signal, new ballistae::dense_signal<double>(std::move(sig)));
}

ballistae::dense_signal<double> signal_from_scm(SCM obj)
{
    ensure_smob(obj, flag_dense_signal);
    return *smob_get_data<ballistae::dense_signal<double>*>(obj);
}

size_t signal_free(SCM obj)
{
    auto p = smob_get_data<ballistae::dense_signal<double>*>(obj);
    delete p;
    return 0;
}

int signal_print(SCM obj, SCM port, scm_print_state *pstate)
{
    SCM fmt = scm_from_utf8_string("#<bsta/dense-signal>");
    scm_simple_format(port, fmt, SCM_EOL);
    return 0;
}

SCM signal_equalp(SCM a, SCM b)
{
    return (signal_from_scm(a) == signal_from_scm(b)) ? SCM_BOOL_T : SCM_BOOL_F;
}

////////////////////////////////////////////////////////////////////////////////
/// Scene subsmob
////////////////////////////////////////////////////////////////////////////////

SCM scene_make()
{
    // The pointer will be deleted when the smob is freed.
    auto scene_p = new ballistae::scene();

    // Materials can rely on the presence of mtlmap1 #0 and #1 for default
    // initialization.

    auto white_up = std::make_unique<ballistae::constant_mtlmap1>(
        ballistae::rgb_to_spectral<double>({0.0, 0.9, 0.0})
    );
    scene_p->mtlmaps_1.push_back(std::move(white_up));

    auto d65_up = std::make_unique<ballistae::constant_mtlmap1>(
        ballistae::cie_d65<double>()
    );
    scene_p->mtlmaps_1.push_back(std::move(d65_up));

    return new_smob(flag_scene, scene_p);
}

ballistae::scene* scene_from_scm(SCM scene)
{
    return smob_get_data<ballistae::scene*>(scene);
}

SCM add_element(SCM scene, SCM geometry, SCM material, SCM transform_scm)
{
    auto transform = daff3_from_scm(transform_scm);

    ballistae::scene_element elt = {
        scene_from_scm(scene)->geometries[scm_to_size_t(geometry)].get(),
        scene_from_scm(scene)->materials[scm_to_size_t(material)].get(),
        inverse(transform),
        transform,
        normal_linear_map(transform)
    };

    size_t index = scene_from_scm(scene)->elements.size();
    scene_from_scm(scene)->elements.push_back(elt);

    return scm_from_size_t(index);
}

SCM set_element_transform(SCM scene, SCM index, SCM transform_scm)
{
    ballistae::scene_element &elt
        = scene_from_scm(scene)->elements[scm_to_size_t(index)];

    auto transform = daff3_from_scm(transform_scm);

    elt.forward_transform = inverse(transform);
    elt.reverse_transform = transform;
    elt.reverse_normal_linear_map = normal_linear_map(transform);

    return index;
}

SCM crush(SCM scene)
{
    return scene;
}

void print_progress_bar(
    std::atomic_bool &stop_flag_atomic,
    std::atomic_size_t &cur_progress_atomic,
    size_t max_progress,
    SCM port
)
{
    while(!stop_flag_atomic.load())
    {
        size_t cur_progress = cur_progress_atomic.load();

        size_t percent = (cur_progress * 100) / max_progress;
        std::cout << '\r' << "Rendering: " << percent << "%" << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\nFinished" << std::endl;
}

SCM render(
    SCM scene,
    SCM camera,
    SCM output_file_scm,
    SCM img_rows_scm,
    SCM img_cols_scm,
    SCM opts_scm
)
{
    scm_dynwind_begin((scm_t_dynwind_flags)0);

    auto output_file = scm_to_utf8_stringn(output_file_scm, nullptr);
    scm_dynwind_free(output_file);
    std::size_t img_rows = scm_to_size_t(img_rows_scm);
    std::size_t img_cols = scm_to_size_t(img_cols_scm);

    SCM sym_gridsize = scm_from_utf8_symbol("gridsize");
    SCM sym_bw_lo = scm_from_utf8_symbol("bw-lo");
    SCM sym_bw_hi = scm_from_utf8_symbol("bw-hi");
    SCM sym_nlambdas = scm_from_utf8_symbol("nlambdas");
    SCM sym_depthlim = scm_from_utf8_symbol("depthlim");

    ballistae::render_opts opts = {
        4,
        {390, 835},
        16,
        16
    };

    SCM lu_gridsize = scm_assq_ref(opts_scm, sym_gridsize);
    SCM lu_bw_lo = scm_assq_ref(opts_scm, sym_bw_lo);
    SCM lu_bw_hi = scm_assq_ref(opts_scm, sym_bw_hi);
    SCM lu_nlambdas = scm_assq_ref(opts_scm, sym_nlambdas);
    SCM lu_depthlim = scm_assq_ref(opts_scm, sym_depthlim);

    if(scm_is_true(lu_gridsize))
        opts.gridsize = scm_to_size_t(lu_gridsize);
    if(scm_is_true(lu_bw_lo))
        opts.bandwidth.lo() = scm_to_double(lu_bw_lo);
    if(scm_is_true(lu_bw_hi))
        opts.bandwidth.hi() = scm_to_double(lu_bw_hi);
    if(scm_is_true(lu_nlambdas))
        opts.n_lambdas = scm_to_size_t(lu_nlambdas);
    if(scm_is_true(lu_depthlim))
        opts.depth_lim = scm_to_size_t(lu_depthlim);

    //
    // Below this point, no scheme exceptions may be thrown.
    //

    // Launch the progress bar.
    std::atomic_bool stop_flag;
    stop_flag = false;
    std::atomic_size_t cur_progress;
    cur_progress = 0;
    auto progress_bar_future = std::async(
        std::launch::async,
        std::bind(
            print_progress_bar,
            std::ref(stop_flag),
            std::ref(cur_progress),
            img_rows,
            scm_current_output_port()
        )
    );

    ballistae::image<float> hdr_img = ballistae::render_scene(
        img_rows,
        img_cols,
        *camera_from_scm(camera),
        *scene_from_scm(scene),
        opts,
        cur_progress
    );

    // Stop the progress printer.
    stop_flag = true;

    if(ballistae::write_pfm(hdr_img, output_file) != 0)
    {
        scm_simple_format(
            SCM_BOOL_T,
            scm_from_utf8_string("Warning: could not write output file."),
            SCM_EOL
        );
    }

    // Wait for the progress bar to finish
    progress_bar_future.get();

    scm_dynwind_end();

    return SCM_BOOL_F;
}

size_t scene_free(SCM obj)
{
    auto scene = smob_get_data<ballistae::scene*>(obj);
    delete scene;
    return 0;
}

int scene_print(SCM obj, SCM port, scm_print_state *pstate)
{
    SCM format_string = scm_from_utf8_string(
        "#<ballistae/scene >"
    );

    scm_simple_format(port, format_string, SCM_EOL);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Material maps that produce scalars.
////////////////////////////////////////////////////////////////////////////////
///
/// Material maps are wholly-owned by the scene.  Guile code can only access
/// them via index into the scene.

SCM mtlmap1_create_constant(SCM scene, SCM config)
{
    auto up = std::make_unique<ballistae::constant_mtlmap1>(
        ballistae::smits_white<double>()
    );

    SCM sym_spectrum = scm_from_utf8_symbol("spectrum");
    SCM lu_spectrum = scm_assq_ref(config, sym_spectrum);

    if(scm_is_true(sym_spectrum))
        up->spectrum = signal_from_scm(lu_spectrum);

    size_t index = scene_from_scm(scene)->mtlmaps_1.size();
    scene_from_scm(scene)->mtlmaps_1.push_back(std::move(up));

    return scm_from_size_t(index);
}

SCM mtlmap1_create_lerp(SCM scene, SCM config)
{
    auto up = std::make_unique<ballistae::lerp_mtlmap1>(
        0.0,
        1.0,
        scene_from_scm(scene)->mtlmaps_1[0].get(),
        scene_from_scm(scene)->mtlmaps_1[0].get(),
        scene_from_scm(scene)->mtlmaps_1[0].get()
    );

    SCM sym_t_lo = scm_from_utf8_symbol("t-lo");
    SCM sym_t_hi = scm_from_utf8_symbol("t-hi");

    SCM sym_t = scm_from_utf8_symbol("t");
    SCM sym_a = scm_from_utf8_symbol("a");
    SCM sym_b = scm_from_utf8_symbol("b");

    SCM lu_t_lo = scm_assq_ref(config, sym_t_lo);
    SCM lu_t_hi = scm_assq_ref(config, sym_t_hi);
    SCM lu_t = scm_assq_ref(config, sym_t);
    SCM lu_a = scm_assq_ref(config, sym_a);
    SCM lu_b = scm_assq_ref(config, sym_b);

    if(scm_is_true(lu_t_lo))
        up->t_lo = scm_to_double(lu_t_lo);

    if(scm_is_true(lu_t_hi))
        up->t_hi = scm_to_double(lu_t_hi);

    if(scm_is_true(lu_t))
        up->t = scene_from_scm(scene)->mtlmaps_1[scm_to_size_t(lu_t)].get();

    if(scm_is_true(lu_a))
        up->a = scene_from_scm(scene)->mtlmaps_1[scm_to_size_t(lu_a)].get();

    if(scm_is_true(lu_b))
        up->b = scene_from_scm(scene)->mtlmaps_1[scm_to_size_t(lu_b)].get();

    size_t index = scene_from_scm(scene)->mtlmaps_1.size();
    scene_from_scm(scene)->mtlmaps_1.push_back(std::move(up));

    return scm_from_size_t(index);
}

SCM mtlmap1_create_level(SCM scene, SCM config)
{
    auto up = std::make_unique<ballistae::level_mtlmap1>(
        0.5,
        scene_from_scm(scene)->mtlmaps_1[0].get(),
        scene_from_scm(scene)->mtlmaps_1[0].get(),
        scene_from_scm(scene)->mtlmaps_1[0].get()
    );

    SCM sym_t_switch = scm_from_utf8_symbol("t-switch");

    SCM sym_t = scm_from_utf8_symbol("t");
    SCM sym_a = scm_from_utf8_symbol("a");
    SCM sym_b = scm_from_utf8_symbol("b");

    SCM lu_t_switch = scm_assq_ref(config, sym_t_switch);
    SCM lu_t = scm_assq_ref(config, sym_t);
    SCM lu_a = scm_assq_ref(config, sym_a);
    SCM lu_b = scm_assq_ref(config, sym_b);

    if(scm_is_true(lu_t_switch))
        up->t_switch = scm_to_double(lu_t_switch);

    if(scm_is_true(lu_t))
        up->t = scene_from_scm(scene)->mtlmaps_1[scm_to_size_t(lu_t)].get();

    if(scm_is_true(lu_a))
        up->a = scene_from_scm(scene)->mtlmaps_1[scm_to_size_t(lu_a)].get();

    if(scm_is_true(lu_b))
        up->b = scene_from_scm(scene)->mtlmaps_1[scm_to_size_t(lu_b)].get();

    size_t index = scene_from_scm(scene)->mtlmaps_1.size();
    scene_from_scm(scene)->mtlmaps_1.push_back(std::move(up));

    return scm_from_size_t(index);
}

SCM mtlmap1_create_checkerboard(SCM scene, SCM config)
{
    auto up = std::make_unique<ballistae::checkerboard_mtlmap1>(
        1.0,
        true
    );

    SCM sym_period = scm_from_utf8_symbol("period");
    SCM sym_volumetric = scm_from_utf8_symbol("volumetric");

    SCM lu_period      = scm_assq_ref(config, sym_period);

    // Boolean values need to be looked up differently.
    SCM lu_volumetric_cons  = scm_assq(sym_volumetric, config);

    if(scm_is_true(lu_period))
        up->period = scm_to_double(lu_period);

    if(scm_is_true(lu_volumetric_cons))
        up->volumetric = scm_is_true(scm_cdr(lu_volumetric_cons));

    size_t index = scene_from_scm(scene)->mtlmaps_1.size();
    scene_from_scm(scene)->mtlmaps_1.push_back(std::move(up));

    return scm_from_size_t(index);
}

SCM mtlmap1_create_bullseye(SCM scene, SCM config)
{
    auto up = std::make_unique<ballistae::bullseye_mtlmap1>(
        1.0,
        true
    );

    SCM sym_period = scm_from_utf8_symbol("period");
    SCM sym_volumetric = scm_from_utf8_symbol("volumetric");

    SCM lu_period      = scm_assq_ref(config, sym_period);
    // Boolean values need to be looked up differently.
    SCM lu_volumetric_cons  = scm_assq(sym_volumetric, config);

    if(scm_is_true(lu_period))
        up->period = scm_to_double(lu_period);

    if(scm_is_true(lu_volumetric_cons))
        up->volumetric = scm_is_true(scm_cdr(lu_volumetric_cons));

    size_t index = scene_from_scm(scene)->mtlmaps_1.size();
    scene_from_scm(scene)->mtlmaps_1.push_back(std::move(up));

    return scm_from_size_t(index);
}

SCM mtlmap1_create_perlinval(SCM scene, SCM config)
{
    auto up = std::make_unique<ballistae::perlinval_mtlmap1>(
        true,
        1.0
    );

    SCM sym_volumetric = scm_from_utf8_symbol("volumetric");
    // Boolean values need to be looked up differently.
    SCM lu_volumetric_cons  = scm_assq(sym_volumetric, config);

    SCM sym_period = scm_from_utf8_symbol("period");
    SCM lu_period = scm_assq_ref(config, sym_period);

    if(scm_is_true(lu_volumetric_cons))
        up->volumetric = scm_is_true(scm_cdr(lu_volumetric_cons));

    if(scm_is_true(lu_period))
        up->period = scm_to_double(lu_period);

    size_t index = scene_from_scm(scene)->mtlmaps_1.size();
    scene_from_scm(scene)->mtlmaps_1.push_back(std::move(up));

    return scm_from_size_t(index);
}

SCM mtlmap1_create_plugin(SCM scene, SCM create_fn, SCM config)
{
    return SCM_UNDEFINED;
}

////////////////////////////////////////////////////////////////////////////////
/// Material maps that produce vectors.
////////////////////////////////////////////////////////////////////////////////

SCM mtlmap2_create_plugin(SCM scene, SCM create_fn, SCM config)
{
    return SCM_UNDEFINED;
}

////////////////////////////////////////////////////////////////////////////////
/// Materials
////////////////////////////////////////////////////////////////////////////////
///
/// Materials are wholly-owned by the scene, and are manipulated from guile by
/// indices into the scene.

SCM material_plugin(SCM scene, SCM create_fn_scm, SCM config)
{
    auto void_fn = scm_to_pointer(create_fn_scm);
    auto create_fn = reinterpret_cast<create_material_t>(void_fn);

    auto up_um = create_fn(scene_from_scm(scene), config);
    auto up_m = std::unique_ptr<ballistae::material>(std::move(up_um));

    size_t index = scene_from_scm(scene)->materials.size();
    scene_from_scm(scene)->materials.push_back(std::move(up_m));

    return scm_from_size_t(index);
}

SCM material_update(SCM scene, SCM index, SCM config)
{
    ballistae::material *p_matr = scene_from_scm(scene)->materials[scm_to_size_t(index)].get();
    auto p_updatable = dynamic_cast<updatable_material*>(p_matr);
    p_updatable->guile_update(scene_from_scm(scene), config);

    return index;
}

////////////////////////////////////////////////////////////////////////////////
/// Geometry.
////////////////////////////////////////////////////////////////////////////////
///
/// Geometry is wholly-owned by the scene.  Guile code interacts with geometry
/// by index.

SCM geometry_plugin(SCM scene, SCM create_fn_scm, SCM config)
{
    auto void_fn = scm_to_pointer(create_fn_scm);
    auto create_fn = reinterpret_cast<create_geometry_t>(void_fn);

    auto up_g = create_fn(scene_from_scm(scene), config);

    size_t idx = scene_from_scm(scene)->geometries.size();
    scene_from_scm(scene)->geometries.push_back(std::move(up_g));

    return scm_from_size_t(idx);
}

////////////////////////////////////////////////////////////////////////////////
/// Illuminators.
////////////////////////////////////////////////////////////////////////////////
///
/// Illuminators are wholly-owned by the scene.  Guile code interacts with
/// illuminators by index.

SCM illuminator_directional(SCM scene, SCM config_alist)
{
    SCM sym_spectrum = scm_from_utf8_symbol("spectrum");
    SCM sym_direction = scm_from_utf8_symbol("direction");

    SCM lu_spectrum = scm_assq_ref(config_alist, sym_spectrum);
    SCM lu_direction = scm_assq_ref(config_alist, sym_direction);

    auto p = std::make_unique<ballistae::dir_illuminator>();

    p->spectrum = signal_from_scm(lu_spectrum);
    p->direction = dvec3_from_scm(lu_direction);

    p->direction = normalise(p->direction);

    size_t index = scene_from_scm(scene)->illuminators.size();
    scene_from_scm(scene)->illuminators.push_back(std::move(p));
    return scm_from_size_t(index);
}

SCM illuminator_point(SCM scene, SCM config_alist)
{
    SCM sym_spectrum = scm_from_utf8_symbol("spectrum");
    SCM sym_position = scm_from_utf8_symbol("position");

    SCM lu_spectrum = scm_assq_ref(config_alist, sym_spectrum);
    SCM lu_position = scm_assq_ref(config_alist, sym_position);

    auto p = std::make_unique<ballistae::point_illuminator>();

    p->spectrum = signal_from_scm(lu_spectrum);
    p->position = dvec3_from_scm(lu_position);

    size_t index = scene_from_scm(scene)->illuminators.size();
    scene_from_scm(scene)->illuminators.push_back(std::move(p));
    return scm_from_size_t(index);
}

SCM illuminator_plugin(SCM scene, SCM create_fn_scm, SCM config)
{
    auto void_fn = scm_to_pointer(create_fn_scm);
    auto create_fn = reinterpret_cast<create_illuminator_t>(void_fn);

    auto p = create_fn(scene, config);

    size_t idx = scene_from_scm(scene)->illuminators.size();
    scene_from_scm(scene)->illuminators.push_back(std::move(p));

    return scm_from_size_t(idx);
}

////////////////////////////////////////////////////////////////////////////////
/// Subsmob dispatch machinery.
////////////////////////////////////////////////////////////////////////////////

using smob_free_t = size_t (*)(SCM);
using smob_print_t = int (*)(SCM, SCM, scm_print_state*);
using smob_equalp_t = SCM (*)(SCM, SCM);

struct subsmob_fns
{
    smob_free_t   free_fn;
    smob_print_t  print_fn;
    smob_equalp_t equalp_fn;
};

static subsmob_fns subsmob_dispatch_table [] = {
    {&scene_free, &scene_print, nullptr},
    {&camera_free, &camera_print, nullptr},
    {&signal_free, &signal_print, &signal_equalp}
};

////////////////////////////////////////////////////////////////////////////////
/// Forwarders for essential subsmob functions.
////////////////////////////////////////////////////////////////////////////////

size_t smob_free(SCM obj)
{
    scm_t_bits flags = get_subsmob_type(obj);
    smob_free_t free_fn = subsmob_dispatch_table[flags].free_fn;
    return (*free_fn)(obj);
}

SCM smob_mark(SCM obj)
{
    return SCM_BOOL_F;
}

int smob_print(SCM obj, SCM port, scm_print_state *pstate)
{
    scm_t_bits flags = get_subsmob_type(obj);
    smob_print_t print_fn = subsmob_dispatch_table[flags].print_fn;
    return (*print_fn)(obj, port, pstate);
}

SCM smob_equalp(SCM a, SCM b)
{
    // The equalp forwarder is a little more complex -- it needs to ensure the
    // two provided smobs are of the same subsmob type before dispatching.

    scm_t_bits flags_a = get_subsmob_type(a);
    scm_t_bits flags_b = get_subsmob_type(b);

    if(flags_a != flags_b)
    {
        return SCM_BOOL_F;
    }
    else
    {
        smob_equalp_t equalp_fn = subsmob_dispatch_table[flags_a].equalp_fn;
        if(nullptr == equalp_fn)
            return SCM_BOOL_F;
        else
            return (*equalp_fn)(a, b);
    }
}

}

extern "C" void libguile_ballistae_init()
{
    using namespace ballistae_guile;

    // Initialize the base smob type.
    //
    // The 0 parameter indicates that there is no automatically-managed memory
    // attached to the smob type.  We *must* then provide free/mark/print/equalp
    // functions for the smob.
    ballistae_guile_smob_tag = scm_make_smob_type("ballistae", 0);

    scm_set_smob_free(  ballistae_guile_smob_tag, &smob_free  );
    scm_set_smob_mark(  ballistae_guile_smob_tag, &smob_mark  );
    scm_set_smob_print( ballistae_guile_smob_tag, &smob_print );
    scm_set_smob_equalp(ballistae_guile_smob_tag, &smob_equalp);

    scm_c_define_gsubr("bsta/backend/cam/make", 2, 0, 0, (scm_t_subr) camera_make);

    scm_c_define_gsubr("bsta/backend/signal/from-list",       3, 0, 0, (scm_t_subr) signal_from_list);
    scm_c_define_gsubr("bsta/backend/signal/from-fn",         4, 0, 0, (scm_t_subr) signal_from_function);
    scm_c_define_gsubr("bsta/backend/signal/pulse",           3, 0, 0, (scm_t_subr) pulse);
    scm_c_define_gsubr("bsta/backend/signal/red",             1, 0, 0, (scm_t_subr) red);
    scm_c_define_gsubr("bsta/backend/signal/green",           1, 0, 0, (scm_t_subr) green);
    scm_c_define_gsubr("bsta/backend/signal/blue",            1, 0, 0, (scm_t_subr) blue);
    scm_c_define_gsubr("bsta/backend/signal/sunlight",        1, 0, 0, (scm_t_subr) sunlight);
    scm_c_define_gsubr("bsta/backend/signal/cie-a",           0, 0, 0, (scm_t_subr) cie_a);
    scm_c_define_gsubr("bsta/backend/signal/cie-d65",         0, 0, 0, (scm_t_subr) cie_d65);
    scm_c_define_gsubr("bsta/backend/signal/rgb-to-spectral", 3, 0, 0, (scm_t_subr) rgb_to_spectral);

    scm_c_define_gsubr("bsta/backend/geom/plugin", 3, 0, 0, (scm_t_subr) geometry_plugin);

    scm_c_define_gsubr("bsta/backend/illum/directional", 2, 0, 0, (scm_t_subr) illuminator_directional);
    scm_c_define_gsubr("bsta/backend/illum/point",       2, 0, 0, (scm_t_subr) illuminator_point);
    scm_c_define_gsubr("bsta/backend/illum/plugin",      3, 0, 0, (scm_t_subr) illuminator_plugin);

    scm_c_define_gsubr("bsta/backend/matr/plugin", 3, 0, 0, (scm_t_subr) material_plugin);
    scm_c_define_gsubr("bsta/backend/matr/update", 3, 0, 0, (scm_t_subr) material_update);

    scm_c_define_gsubr("bsta/backend/scene/make",            0, 0, 0, (scm_t_subr) scene_make);
    scm_c_define_gsubr("bsta/backend/scene/add-element",     4, 0, 0, (scm_t_subr) add_element);
    scm_c_define_gsubr("bsta/backend/scene/set-element-transform", 3, 0, 0, (scm_t_subr) set_element_transform);
    scm_c_define_gsubr("bsta/backend/scene/crush",           1, 0, 0, (scm_t_subr) crush);
    scm_c_define_gsubr("bsta/backend/scene/render",          6, 0, 0, (scm_t_subr) render);

    scm_c_define_gsubr("bsta/backend/mtlmap1/constant",      2, 0, 0, (scm_t_subr) mtlmap1_create_constant);
    scm_c_define_gsubr("bsta/backend/mtlmap1/lerp",          2, 0, 0, (scm_t_subr) mtlmap1_create_lerp);
    scm_c_define_gsubr("bsta/backend/mtlmap1/level",         2, 0, 0, (scm_t_subr) mtlmap1_create_level);
    scm_c_define_gsubr("bsta/backend/mtlmap1/checkerboard",  2, 0, 0, (scm_t_subr) mtlmap1_create_checkerboard);
    scm_c_define_gsubr("bsta/backend/mtlmap1/bullseye",      2, 0, 0, (scm_t_subr) mtlmap1_create_bullseye);
    scm_c_define_gsubr("bsta/backend/mtlmap1/perlinval",     2, 0, 0, (scm_t_subr) mtlmap1_create_perlinval);
    scm_c_define_gsubr("bsta/backend/mtlmap1/plugin",        3, 0, 0, (scm_t_subr) mtlmap1_create_plugin);
}
