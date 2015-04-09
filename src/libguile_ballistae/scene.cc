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

#include <libballistae/camera_plugin_interface.hh>
#include <libballistae/geom_plugin_interface.hh>
#include <libballistae/image.hh>
#include <libballistae/matr_plugin_interface.hh>
#include <libballistae/render_scene.hh>
#include <libballistae/scene.hh>

#include <libguile_ballistae/affine_transform.hh>
#include <libguile_ballistae/camera_instance.hh>
#include <libguile_ballistae/geom_instance.hh>
#include <libguile_ballistae/matr_instance.hh>
#include <libguile_ballistae/utility.hh>

namespace ballistae_guile
{

scm_t_bits scene_subsmob_flags;

namespace scene
{

namespace bl = ballistae;

SCM make_backend()
{
    // The pointer will be deleted when the smob is freed.
    auto scene_p = new ballistae::scene();
    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(scene_p));
    SCM_SET_SMOB_FLAGS(result, scene_subsmob_flags);
    return result;
}

bl::scene* p_from_scm(SCM scene)
{
    return smob_get_data<bl::scene*>(scene);
}

SCM add_backend(SCM scene, SCM geometry, SCM material, SCM transform)
{
    auto scene_p = scene::p_from_scm(scene);
    auto geometry_p = geom_instance::sp_from_scm(geometry);
    auto material_p = matr_instance::sp_from_scm(material);
    auto transform_v = affine_transform::from_scm(transform);

    scene_p->geometries.push_back(geometry_p);
    scene_p->materials.push_back(material_p);
    scene_p->trans_for.push_back(bl::inverse(transform_v));
    scene_p->trans_inv.push_back(transform_v);

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
    SCM scene_scm,
    SCM camera_scm,
    SCM output_file_scm,
    SCM img_rows_scm,
    SCM img_cols_scm,
    SCM ss_factor_scm,
    SCM sample_profile_scm,
    SCM lambda_nm_profile_scm
)
{
    scm_dynwind_begin((scm_t_dynwind_flags)0);

    auto cam_p
        = *smob_get_data<std::shared_ptr<ballistae::camera_priv>*>(camera_scm);
    auto the_scene = smob_get_data<ballistae::scene*>(scene_scm);
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
        cam_p,
        *the_scene,
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

    scm_remember_upto_here_1(camera_scm);
    scm_remember_upto_here_1(scene_scm);

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

void init(std::vector<subsmob_fns> &ss_dispatch)
{
    scene_subsmob_flags = ss_dispatch.size();

    scm_c_define_gsubr("bsta/backend/scene/make", 0, 0, 0, (scm_t_subr) make_backend);
    scm_c_define_gsubr("bsta/backend/scene/add", 4, 0, 0, (scm_t_subr) add_backend);
    scm_c_define_gsubr("bsta/backend/scene/crush", 1, 0, 0, (scm_t_subr) crush_backend);
    scm_c_define_gsubr("bsta/backend/scene/render", 8, 0, 0, (scm_t_subr) render_backend);

    ss_dispatch.push_back({&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp});
}

}

}
