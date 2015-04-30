#include <libguile_ballistae/libguile_ballistae.hh>

#include <sys/ioctl.h>
#include <unistd.h>

#include <cstdlib>

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <frustum-0/indicial/fixed.hh>
#include <libguile_frustum0/libguile_frustum0.hh>

#include <libguile_ballistae/camera_plugin_interface.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>
#include <libguile_ballistae/illuminator_plugin_interface.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

static scm_t_bits ballistae_guile_smob_tag;

namespace ballistae_guile
{

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
constexpr scm_t_bits flag_geometry         = 2;
constexpr scm_t_bits flag_material         = 3;
constexpr scm_t_bits flag_illuminator      = 4;
constexpr scm_t_bits flag_affine_transform = 5;
constexpr scm_t_bits flag_dense_signal     = 6;

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
/// Functions related to registration with a scene.
////////////////////////////////////////////////////////////////////////////////

bool is_registered(SCM obj)
{
    return SCM_SMOB_FLAGS(obj) & mask_registered;
}

SCM set_registered(SCM obj, bool registered)
{
    scm_t_bits bits = SCM_SMOB_FLAGS(obj);
    if(registered)
        bits = (bits | mask_registered) & 0xffff;
    else
        bits = (bits & ~mask_registered) & 0xffff;
    SCM_SET_SMOB_FLAGS(obj, bits);
    return obj;
}

////////////////////////////////////////////////////////////////////////////////
/// Affine transform subsmob.
////////////////////////////////////////////////////////////////////////////////

ballistae::affine_transform<double, 3> affine_from_scm(SCM t_scm)
{
    ensure_smob(t_scm, flag_affine_transform);
    return *smob_get_data<ballistae::affine_transform<double, 3>*>(t_scm);
}

SCM compose(SCM rest_scm)
{
    auto total = ballistae::affine_transform<double, 3>::identity() ;

    for(SCM cur = rest_scm; !scm_is_null(cur); cur = scm_cdr(cur))
    {
        SCM aff = ensure_smob(scm_car(cur), flag_affine_transform);
        total = affine_from_scm(aff) * total;
    }

    auto affine_p = new ballistae::affine_transform<double, 3>(total);
    return new_smob(flag_affine_transform, affine_p);
}

SCM identity()
{
    auto affine_p = new ballistae::affine_transform<double, 3>(
        ballistae::affine_transform<double, 3>::identity()
    );

    return new_smob(flag_affine_transform, affine_p);
}

SCM translation(SCM t_scm)
{
    ballistae::fixvec<double, 3> t = guile_frustum::dvec3_from_scm(t_scm);

    auto affine_p = new ballistae::affine_transform<double, 3>(
        ballistae::affine_transform<double, 3>::translation(t)
    );

    scm_remember_upto_here_1(t_scm);

    return new_smob(flag_affine_transform, affine_p);
}

SCM scaling(SCM s_scm)
{
    double s = scm_to_double(s_scm);

    auto affine_p = new ballistae::affine_transform<double, 3>(
        ballistae::affine_transform<double, 3>::scaling(s)
    );

    return new_smob(flag_affine_transform, affine_p);
}

SCM rotation(SCM axis_scm, SCM angle_scm)
{
    ballistae::fixvec<double, 3> axis = guile_frustum::dvec3_from_scm(axis_scm);
    double angle = scm_to_double(angle_scm);

    auto affine_p = new ballistae::affine_transform<double, 3>(
        ballistae::affine_transform<double, 3>::rotation(axis, angle)
    );

    scm_remember_upto_here_1(axis_scm);

    return new_smob(flag_affine_transform, affine_p);
}

SCM basis_mapping(SCM t0_scm, SCM t1_scm, SCM t2_scm)
{
    auto t0 = guile_frustum::dvec3_from_scm(t0_scm);
    auto t1 = guile_frustum::dvec3_from_scm(t1_scm);
    auto t2 = guile_frustum::dvec3_from_scm(t2_scm);

    scm_remember_upto_here_1(t0_scm);
    scm_remember_upto_here_1(t1_scm);
    scm_remember_upto_here_1(t2_scm);

    auto affine_p = new ballistae::affine_transform<double, 3>(
        ballistae::affine_transform<double, 3>::basis_mapping(t0, t1, t2)
    );

    return new_smob(flag_affine_transform, affine_p);
}

size_t affine_free(SCM obj)
{
    auto a_p = smob_get_data<ballistae::affine_transform<double, 3>*>(obj);
    delete a_p;
    return 0;
}

int affine_print(SCM obj, SCM port, scm_print_state *pstate)
{
    SCM fmt = scm_from_utf8_string("#<ballistae/affine_transform>");
    scm_simple_format(port, fmt, SCM_EOL);
    return 0;
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
    p->src_val = scm_to_double(lo);
    p->lim_val = scm_to_double(hi);

    for(; !scm_is_null(val_list); val_list = scm_cdr(val_list))
    {
        p->samples.push_back(scm_to_double(scm_car(val_list)));
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

SCM rgb_to_spectral(SCM red, SCM green, SCM blue)
{
    auto sig = scm_to_double(red) * ballistae::red<double>()
        + scm_to_double(green) * ballistae::green<double>()
        + scm_to_double(blue) * ballistae::blue<double>();

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
/// Geometry subsmob
////////////////////////////////////////////////////////////////////////////////

SCM geometry_make(SCM plug_soname, SCM config_alist)
{
    plug_soname = scm_string_append(
        scm_list_2(
            scm_from_utf8_string("ballistae_geometry_"),
            plug_soname
        )
    );

    SCM so_handle = scm_dynamic_link(plug_soname);

    auto create_fn = reinterpret_cast<create_geometry_t>(
        scm_to_pointer(
            scm_dynamic_pointer(
                scm_from_utf8_string("guile_ballistae_geometry"),
                so_handle
            )
        )
    );

    return new_smob(flag_geometry, create_fn(config_alist));
}

ballistae::geometry* geometry_from_scm(SCM geom)
{
    ensure_smob(geom, flag_geometry);
    return smob_get_data<ballistae::geometry*>(geom);
}

size_t geometry_free(SCM obj)
{
    if(! is_registered(obj))
        delete geometry_from_scm(obj);
    return 0;
}

int geometry_print(SCM obj, SCM port, scm_print_state *pstate)
{
    SCM fmt = scm_from_utf8_string("#<bsta/geometry>");
    scm_simple_format(port, fmt, SCM_EOL);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Illuminator subsmob
////////////////////////////////////////////////////////////////////////////////

SCM dir_illuminator_make(SCM config_alist)
{
    SCM sym_spectrum = scm_from_utf8_symbol("spectrum");
    SCM sym_direction = scm_from_utf8_symbol("direction");

    SCM lu_spectrum = scm_assq_ref(config_alist, sym_spectrum);
    SCM lu_direction = scm_assq_ref(config_alist, sym_direction);

    auto result = new ballistae::dir_illuminator();

    result->spectrum = signal_from_scm(lu_spectrum);
    result->direction = guile_frustum::dvec3_from_scm(lu_direction);

    result->direction = normalise(result->direction);

    return new_smob(flag_illuminator, result);
}

SCM point_isotropic_illuminator_make(SCM config_alist)
{
    SCM sym_spectrum = scm_from_utf8_symbol("spectrum");
    SCM sym_position = scm_from_utf8_symbol("position");

    SCM lu_spectrum = scm_assq_ref(config_alist, sym_spectrum);
    SCM lu_position = scm_assq_ref(config_alist, sym_position);

    auto result = new ballistae::point_isotropic_illuminator();

    result->spectrum = signal_from_scm(lu_spectrum);
    result->position = guile_frustum::dvec3_from_scm(lu_position);

    return new_smob(flag_illuminator, result);
}

SCM illuminator_make(SCM name, SCM config_alist)
{
    if(scm_is_true(scm_equal_p(name, scm_from_utf8_string("dir"))))
        return dir_illuminator_make(config_alist);

    if(scm_is_true(scm_equal_p(name, scm_from_utf8_string("point-isotropic"))))
        return point_isotropic_illuminator_make(config_alist);

    // Otherwise, load from plugin.

    SCM plug_soname = scm_string_append(
        scm_list_2(
            scm_from_utf8_string("ballistae_illuminator_"),
            name
        )
    );

    SCM so_handle = scm_dynamic_link(plug_soname);

    auto create_fn = reinterpret_cast<create_illuminator_t>(
        scm_to_pointer(
            scm_dynamic_pointer(
                scm_from_utf8_string("guile_ballistae_illuminator"),
                so_handle
            )
        )
    );

    return new_smob(flag_illuminator, create_fn(config_alist));
}

ballistae::illuminator* illuminator_from_scm(SCM geom)
{
    ensure_smob(geom, flag_illuminator);
    return smob_get_data<ballistae::illuminator*>(geom);
}

size_t illuminator_free(SCM obj)
{
    if(! is_registered(obj))
        delete illuminator_from_scm(obj);
    return 0;
}

int illuminator_print(SCM obj, SCM port, scm_print_state *pstate)
{
    SCM fmt = scm_from_utf8_string("#<bsta/illuminator>");
    scm_simple_format(port, fmt, SCM_EOL);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Material subsmob
////////////////////////////////////////////////////////////////////////////////

SCM material_make(SCM create_fn_scm, SCM config_alist)
{
    auto void_fn = scm_to_pointer(create_fn_scm);
    auto create_fn = reinterpret_cast<create_material_t>(void_fn);

    return new_smob(flag_material, create_fn(config_alist));
}

ballistae::material* material_from_scm(SCM matr)
{
    ensure_smob(matr, flag_material);
    return smob_get_data<ballistae::material*>(matr);
}

size_t material_free(SCM obj)
{
    if(! is_registered(obj))
        delete material_from_scm(obj);
    return 0;
}

int material_print(SCM obj, SCM port, scm_print_state *pstate)
{
    scm_write_line(scm_from_utf8_string("#<bsta/material>"), SCM_UNDEFINED);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Scene subsmob
////////////////////////////////////////////////////////////////////////////////

SCM scene_make()
{
    // The pointer will be deleted when the smob is freed.
    auto scene_p = new ballistae::scene();
    return new_smob(flag_scene, scene_p);
}

ballistae::scene* scene_from_scm(SCM scene)
{
    return smob_get_data<ballistae::scene*>(scene);
}

/// Functions for registering components into a scene.
///
/// These functions are idempotent, and should be safe against garbage
/// collection running during their extent.

SCM register_geometry(SCM scene, SCM geometry)
{
    if(! is_registered(geometry))
    {
        set_registered(geometry, true);

        auto scene_p = scene_from_scm(scene);
        auto geom_p = geometry_from_scm(geometry);

        scene_p->geometries.push_back(std::unique_ptr<ballistae::geometry>(geom_p));
    }

    return scene;
}

SCM register_material(SCM scene, SCM material)
{
    if(! is_registered(material))
    {
        set_registered(material, true);

        auto scene_p = scene_from_scm(scene);
        auto matr_p = material_from_scm(material);

        scene_p->materials.push_back(std::unique_ptr<ballistae::material>(matr_p));
    }

    return scene;
}

SCM register_illuminator(SCM scene, SCM illuminator)
{
    if(! is_registered(illuminator))
    {
        set_registered(illuminator, true);

        auto scene_p = scene_from_scm(scene);
        auto illum_p = illuminator_from_scm(illuminator);

        scene_p->illuminators.push_back(std::unique_ptr<ballistae::illuminator>(illum_p));
    }

    return scene;
}

SCM add_element(SCM scene, SCM geometry, SCM material, SCM transform)
{
    register_geometry(scene, geometry);
    register_material(scene, material);

    ballistae::scene_element elt = {
        geometry_from_scm(geometry),
        material_from_scm(material),
        ballistae::inverse(affine_from_scm(transform)),
        affine_from_scm(transform)
    };

    scene_from_scm(scene)->elements.push_back(elt);

    return scene;
}

SCM add_illuminator(SCM scene, SCM illuminator)
{
    register_illuminator(scene, illuminator);
    return scene;
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
    std::stringstream valstream;
    while(!stop_flag_atomic.load())
    {
        size_t cur_progress = cur_progress_atomic.load();

        winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        size_t bar_field_width = (size_t) w.ws_col - 2;
        size_t bar_length = (cur_progress * bar_field_width) / max_progress;

        std::string buf(w.ws_col, ' ');
        size_t i = 0;
        buf[i] = '['; ++i;
        for(; i < 1 + bar_length; ++i)
            buf[i] = '=';
        buf[i] = '>'; ++i;
        for(; i < 1 + bar_field_width; ++i)
            buf[i] = ' ';
        buf[i] = ']';

        std::cout << '\r' << buf << std::flush;

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
        opts.bandwidth.lo = scm_to_double(lu_bw_lo);
    if(scm_is_true(lu_bw_hi))
        opts.bandwidth.hi = scm_to_double(lu_bw_hi);
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
    {&geometry_free, &geometry_print, nullptr},
    {&material_free, &material_print, nullptr},
    {&illuminator_free, &illuminator_print, nullptr},
    {&affine_free, &affine_print, nullptr},
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

    scm_c_define_gsubr("bsta/backend/aff-t/identity",      0, 0, 0, (scm_t_subr) identity);
    scm_c_define_gsubr("bsta/backend/aff-t/translation",   1, 0, 0, (scm_t_subr) translation);
    scm_c_define_gsubr("bsta/backend/aff-t/scaling",       1, 0, 0, (scm_t_subr) scaling);
    scm_c_define_gsubr("bsta/backend/aff-t/rotation",      2, 0, 0, (scm_t_subr) rotation);
    scm_c_define_gsubr("bsta/backend/aff-t/basis-mapping", 3, 0, 0, (scm_t_subr) basis_mapping);
    scm_c_define_gsubr("bsta/backend/aff-t/compose",       0, 0, 1, (scm_t_subr) compose);

    scm_c_define_gsubr("bsta/backend/cam/make", 2, 0, 0, (scm_t_subr) camera_make);

    scm_c_define_gsubr("bsta/backend/signal/from-list",       3, 0, 0, (scm_t_subr) signal_from_list);
    scm_c_define_gsubr("bsta/backend/signal/pulse",           3, 0, 0, (scm_t_subr) pulse);
    scm_c_define_gsubr("bsta/backend/signal/red",             1, 0, 0, (scm_t_subr) red);
    scm_c_define_gsubr("bsta/backend/signal/green",           1, 0, 0, (scm_t_subr) green);
    scm_c_define_gsubr("bsta/backend/signal/blue",            1, 0, 0, (scm_t_subr) blue);
    scm_c_define_gsubr("bsta/backend/signal/rgb-to-spectral", 3, 0, 0, (scm_t_subr) rgb_to_spectral);

    scm_c_define_gsubr("bsta/backend/geom/make", 2, 0, 0, (scm_t_subr) geometry_make);

    scm_c_define_gsubr("bsta/backend/illum/make", 2, 0, 0, (scm_t_subr) illuminator_make);

    scm_c_define_gsubr("bsta/backend/matr/make", 2, 0, 0, (scm_t_subr) material_make);

    scm_c_define_gsubr("bsta/backend/scene/make",            0, 0, 0, (scm_t_subr) scene_make);
    scm_c_define_gsubr("bsta/backend/scene/add-element",     4, 0, 0, (scm_t_subr) add_element);
    scm_c_define_gsubr("bsta/backend/scene/add-illuminator", 2, 0, 0, (scm_t_subr) add_illuminator);
    scm_c_define_gsubr("bsta/backend/scene/crush",           1, 0, 0, (scm_t_subr) crush);
    scm_c_define_gsubr("bsta/backend/scene/render",          6, 0, 0, (scm_t_subr) render);
}
