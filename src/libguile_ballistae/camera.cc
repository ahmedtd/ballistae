#include <libguile_ballistae/camera.hh>

#include <memory>
#include <vector>

#include <libballistae/camera.hh>

#include <libguile_ballistae/libguile_ballistae.hh>
#include <libguile_ballistae/camera_plugin_interface.hh>

namespace ballistae_guile
{

namespace camera
{

SCM make(SCM plug_name, SCM config_alist)
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

bl::camera* p_from_scm(SCM obj)
{
    ensure_smob(obj, flag_camera);
    return smob_get_data<bl::camera*>(obj);
}

size_t subsmob_free(SCM obj)
{
    delete p_from_scm(obj);
    return 0;
}

SCM subsmob_mark(SCM obj)
{
    return SCM_BOOL_F;
}

int subsmob_print(SCM obj, SCM port, scm_print_state *pstate)
{
    scm_write_line(scm_from_utf8_string("#<bsta/camera>"), SCM_UNDEFINED);
    return 0;
}

SCM subsmob_equalp(SCM a, SCM b)
{
    return (p_from_scm(a) == p_from_scm(b)) ? SCM_BOOL_T : SCM_BOOL_F;
}

subsmob_fns init()
{
    scm_c_define_gsubr("bsta/backend/cam/make", 2, 0, 0, (scm_t_subr) make);

    return {&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp};
}

}

}
