#include <libguile_ballistae/scene.hh>

#include <memory>
#include <utility>

#include <cstddef> // workaround for bug in GMP
#include <libguile.h>

#include <armadillo>

#define cimg_display 0
#define cimg_verbosity 1
#include <CImg.h>

#include <libballistae/camera_plugin_interface.hh>
#include <libballistae/geom_plugin_interface.hh>
#include <libballistae/matr_plugin_interface.hh>
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

void init(std::vector<subsmob_fns> &ss_dispatch)
{
    scene_subsmob_flags = ss_dispatch.size();

    scm_c_define_gsubr("ballistae/scene/crush", 2, 0, 0, (scm_t_subr) crush);
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

SCM crush(SCM infty_matr, SCM geometry_material_alist)
{
    constexpr const char* const subr = "ballistae::scene::crush";

    SCM_ASSERT_TYPE(
        scm_is_true(matr_instance::matr_p(infty_matr)),
        infty_matr,
        SCM_ARG1,
        subr,
        "ballistae/matr"
    );

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

    the_scene->infty_matr
        = *smob_get_data<std::shared_ptr<ballistae::matr_priv>*>(infty_matr);

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

    namespace cimg = cimg_library;

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

    cimg::CImg<float> hdr_img(img_rows, img_cols, 1, 3);

    hdr_img = ballistae::render_scene(
        hdr_img,
        cam_p,
        *the_scene,
        ss_factor
    );

    // Convert to LDR image.
    hdr_img /= std::max(hdr_img.max(), 1.0f);
    hdr_img *= std::nextafter(256.0f, 0.0f);
    cimg::CImg<std::uint8_t> ldr_img = hdr_img;

    ldr_img.save_jpeg(output_file);

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
