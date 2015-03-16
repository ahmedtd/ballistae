#include <libguile_ballistae/scene.hh>

#include <cstdlib>

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
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

void init(std::vector<subsmob_fns> &ss_dispatch)
{
    scene_subsmob_flags = ss_dispatch.size();

    scm_c_define_gsubr("ballistae/scene/crush", 1, 0, 0, (scm_t_subr) crush);
    scm_c_define_gsubr("ballistae/render-scene", 6, 0, 0, (scm_t_subr) render_scene);
    scm_c_define_gsubr("ballistae/scene?", 1, 0, 0, (scm_t_subr) scene_p);

    scm_c_export(
        "ballistae/scene/crush",
        "ballistae/render-scene",
        "ballistae/scene?",
        nullptr
    );

    ss_dispatch.push_back({&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp});
}

SCM crush(SCM geometry_material_alist)
{
    constexpr const char* const subr = "ballistae::scene::crush";

    // Verify the geometry.
    SCM cur_head = geometry_material_alist;
    while(!scm_is_null(cur_head))
    {
        SCM cur_geom = scm_caar(cur_head);
        SCM cur_matr = scm_cdar(cur_head);
        cur_head = scm_cdr(cur_head);

        SCM_ASSERT_TYPE(
            scm_is_true(geom_instance::geom_p(cur_geom)),
            cur_geom,
            SCM_ARGn,
            subr,
            "ballistae/geom"
        );

        SCM_ASSERT_TYPE(
            scm_is_true(matr_instance::matr_p(cur_matr)),
            cur_matr,
            SCM_ARGn,
            subr,
            "ballistae/matr"
        );
    }

    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(nullptr));
    SCM_SET_SMOB_FLAGS(result, scene_subsmob_flags);

    //
    // Below this point, no code may result in a scheme error, since we rely on
    // c++ stack unwinding.
    //

    auto the_scene = new ballistae::scene();

    cur_head = geometry_material_alist;
    while(!scm_is_null(cur_head))
    {
        SCM cur_geom = scm_caar(cur_head);
        SCM cur_matr = scm_cdar(cur_head);
        cur_head = scm_cdr(cur_head);

        the_scene->materials.push_back(
            *smob_get_data<std::shared_ptr<ballistae::matr_priv>*>(cur_matr)
        );

        the_scene->geometries.push_back(
            *smob_get_data<std::shared_ptr<ballistae::geom_priv>*>(cur_geom)
        );
    }

    smob_set_data(result, the_scene);

    return result;
}

void print_progress_bar(
    std::atomic_bool &stop_flag_atomic,
    std::atomic_size_t &cur_progress_atomic,
    size_t max_progress,
    SCM port
)
{
    using namespace std::literals::chrono_literals;

    // if(scm_is_true(scm_isatty_p(scm_current_output_port())))
    // {
    //     while(!stop_flag_atomic.load())
    //     {
    //         std::this_thread::sleep_for(1s);
    //     }

    //     return;
    // }

    SCM carriage_return = scm_from_utf8_string("\r");
    SCM open_bracket = scm_from_utf8_string("[");
    SCM close_bracket = scm_from_utf8_string("]");
    SCM equals_sign = scm_from_utf8_string("=");
    SCM close_angbracket = scm_from_utf8_string(">");
    SCM space = scm_from_utf8_string(" ");

    std::stringstream valstream;
    while(!stop_flag_atomic.load())
    {
        size_t cur_progress = cur_progress_atomic.load();

        const char *colvar = std::getenv("COLUMNS");
        size_t term_width = 0;
        if(colvar != nullptr)
            term_width = std::strtoul(colvar, nullptr, 10);
        if(term_width < 80)
            term_width = 80;

        size_t bar_field_width = term_width - 2;
        size_t bar_length = (cur_progress * bar_field_width) / max_progress;

        // Write progress bar.
        scm_simple_format(port, carriage_return, SCM_EOL);
        scm_simple_format(port, open_bracket, SCM_EOL);
        for(size_t i = 0; i < bar_length; ++i)
        {
            scm_simple_format(port, equals_sign, SCM_EOL);
        }
        scm_simple_format(port, close_angbracket, SCM_EOL);
        for(size_t i = bar_length + 1; i < bar_field_width; ++i)
        {
            scm_simple_format(port, space, SCM_EOL);
        }
        scm_simple_format(port, close_bracket, SCM_EOL);

        scm_force_output(port);

        std::this_thread::sleep_for(1s);
    }

    scm_simple_format(port, scm_from_utf8_string("\nFinished.\n"), SCM_EOL);
    scm_force_output(port);
}

SCM render_scene(
    SCM camera_scm,
    SCM scene_scm,
    SCM output_file_scm,
    SCM img_rows_scm,
    SCM img_cols_scm,
    SCM ss_factor_scm
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
        cur_progress
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

SCM scene_p(SCM obj)
{
    return scm_from_bool(
        scm_is_true(ballistae_p(obj))
        && SCM_SMOB_FLAGS(obj) == scene_subsmob_flags
    );
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

}

}
