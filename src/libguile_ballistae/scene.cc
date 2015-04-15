#include <libguile_ballistae/scene.hh>

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
#include <utility>

#include <cstddef> // workaround for bug in GMP
#include <libguile.h>

#include <armadillo>

#include <libballistae/camera.hh>
#include <libballistae/geometry.hh>
#include <libballistae/illuminator.hh>
#include <libballistae/image.hh>
#include <libballistae/material.hh>
#include <libballistae/render_scene.hh>
#include <libballistae/scene.hh>

#include <libguile_ballistae/libguile_ballistae.hh>
#include <libguile_ballistae/affine_transform.hh>
#include <libguile_ballistae/camera.hh>
#include <libguile_ballistae/geometry.hh>
#include <libguile_ballistae/illuminator.hh>
#include <libguile_ballistae/material.hh>

namespace ballistae_guile
{

namespace scene
{

SCM make_backend()
{
    // The pointer will be deleted when the smob is freed.
    auto scene_p = new ballistae::scene();
    return new_smob(flag_scene, scene_p);
}

bl::scene* p_from_scm(SCM scene)
{
    return smob_get_data<bl::scene*>(scene);
}

////////////////////////////////////////////////////////////////////////////////
/// Functions for registering components into a scene.
////////////////////////////////////////////////////////////////////////////////
///
/// These functions are idempotent, and should be safe against garbage
/// collection running during their extent.

SCM register_geometry(SCM scene, SCM geometry)
{
    if(! is_registered(geometry))
    {
        set_registered(geometry, true);

        auto scene_p = scene::p_from_scm(scene);
        auto geom_p = geometry::p_from_scm(geometry);

        scene_p->geometries.push_back(std::unique_ptr<bl::geometry>(geom_p));
    }

    return scene;
}

SCM register_material(SCM scene, SCM material)
{
    if(! is_registered(material))
    {
        set_registered(material, true);

        auto scene_p = scene::p_from_scm(scene);
        auto matr_p = material::p_from_scm(material);

        scene_p->materials.push_back(std::unique_ptr<bl::material>(matr_p));
    }

    return scene;
}

SCM register_illuminator(SCM scene, SCM illuminator)
{
    if(! is_registered(illuminator))
    {
        set_registered(illuminator, true);

        auto scene_p = scene::p_from_scm(scene);
        auto illum_p = illuminator::p_from_scm(illuminator);

        scene_p->illuminators.push_back(std::unique_ptr<bl::illuminator>(illum_p));
    }

    return scene;
}

SCM add_element(SCM scene, SCM geometry, SCM material, SCM transform)
{
    register_geometry(scene, geometry);
    register_material(scene, material);

    bl::scene_element elt = {
        geometry::p_from_scm(geometry),
        material::p_from_scm(material),
        bl::inverse(affine_transform::from_scm(transform)),
        affine_transform::from_scm(transform)
    };

    scene::p_from_scm(scene)->elements.push_back(elt);

    return scene;
}

SCM add_illuminator(SCM scene, SCM illuminator)
{
    register_illuminator(scene, illuminator);
    return scene;
}

SCM crush_backend(SCM scene)
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

SCM render_backend(
    SCM scene,
    SCM camera,
    SCM output_file_scm,
    SCM img_rows_scm,
    SCM img_cols_scm,
    SCM ss_factor_scm,
    SCM sample_profile_scm,
    SCM lambda_nm_profile_scm
)
{
    scm_dynwind_begin((scm_t_dynwind_flags)0);

    auto output_file = scm_to_utf8_stringn(output_file_scm, nullptr);
    scm_dynwind_free(output_file);
    std::size_t img_rows = scm_to_size_t(img_rows_scm);
    std::size_t img_cols = scm_to_size_t(img_cols_scm);
    std::size_t ss_factor = scm_to_size_t(ss_factor_scm);

    std::vector<size_t> sample_profile;
    for(SCM cur = sample_profile_scm; !scm_is_null(cur); cur = scm_cdr(cur))
    {
        sample_profile.push_back(scm_to_size_t(scm_car(cur)));
    }

    std::vector<double> lambda_nm_profile;
    for(SCM cur = lambda_nm_profile_scm; !scm_is_null(cur); cur = scm_cdr(cur))
    {
        lambda_nm_profile.push_back(scm_to_double(scm_car(cur)));
    }

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

    bl::image<float> hdr_img = ballistae::render_scene(
        img_rows,
        img_cols,
        *camera::p_from_scm(camera),
        *scene::p_from_scm(scene),
        ss_factor,
        cur_progress,
        sample_profile,
        lambda_nm_profile
    );

    // Stop the progress printer.
    stop_flag = true;

    if(bl::write_pfm(hdr_img, output_file) != 0)
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

size_t subsmob_free(SCM obj)
{
    auto scene = smob_get_data<ballistae::scene*>(obj);
    delete scene;
    return 0;
}

SCM subsmob_mark(SCM obj)
{
    return SCM_BOOL_F;
}

int subsmob_print(SCM obj, SCM port, scm_print_state *pstate)
{
    SCM format_string = scm_from_utf8_string(
        "#<ballistae/scene >"
    );

    scm_simple_format(port, format_string, SCM_EOL);
    return 0;
}

SCM subsmob_equalp(SCM a, SCM b)
{
    auto a_p = smob_get_data<ballistae::scene*>(a);
    auto b_p = smob_get_data<ballistae::scene*>(b);

    return scm_from_bool(a_p == b_p);
}

subsmob_fns init()
{
    scm_c_define_gsubr("bsta/backend/scene/make", 0, 0, 0, (scm_t_subr) make_backend);
    scm_c_define_gsubr("bsta/backend/scene/add-element", 4, 0, 0, (scm_t_subr) add_element);
    scm_c_define_gsubr("bsta/backend/scene/add-illuminator", 2, 0, 0, (scm_t_subr) add_illuminator);
    scm_c_define_gsubr("bsta/backend/scene/crush", 1, 0, 0, (scm_t_subr) crush_backend);
    scm_c_define_gsubr("bsta/backend/scene/render", 8, 0, 0, (scm_t_subr) render_backend);

    return {&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp};
}

}

}
