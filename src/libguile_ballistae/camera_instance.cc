#include <libguile_ballistae/camera_instance.hh>

#include <memory>
#include <vector>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/camera_plugin_interface.hh>

#include <libguile_ballistae/camera_plugin_interface.hh>
#include <libguile_ballistae/utility.hh>

namespace ballistae_guile
{

scm_t_bits camera_subsmob_flags;

namespace camera_instance
{

void init(std::vector<subsmob_fns> &ss_dispatch)
{
    camera_subsmob_flags = ss_dispatch.size();

    scm_c_define_gsubr("ballistae/camera", 2, 0, 0, (scm_t_subr) make);
    scm_c_define_gsubr("ballistae/camera?", 1, 0, 0, (scm_t_subr) camera_p);

    scm_c_export(
        "ballistae/camera",
        "ballistae/camera?",
        nullptr
    );

    ss_dispatch.push_back({&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp});
}

SCM make(SCM plug_name, SCM config_alist)
{
    namespace b = ballistae;

    SCM plug_soname = scm_string_append(
        scm_list_2(
            scm_from_utf8_string("ballistae_camera_plugin_"),
            plug_name
        )
    );

    SCM so_handle = scm_dynamic_link(plug_soname);

    auto create_fn = reinterpret_cast<camera_create_from_alist_t>(
        scm_to_pointer(
            scm_dynamic_pointer(
                scm_from_utf8_string("ballistae_camera_create_from_alist"),
                so_handle
            )
        )
    );

    auto camera_p = new std::shared_ptr<ballistae::camera_priv>(
        create_fn(config_alist)
    );

    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(camera_p));
    SCM_SET_SMOB_FLAGS(result, camera_subsmob_flags);
    return result;
}

SCM camera_p(SCM obj)
{
    return scm_from_bool(
        scm_is_true(ballistae_p(obj))
        && SCM_SMOB_FLAGS(obj) == camera_subsmob_flags
    );
}

size_t subsmob_free(SCM obj)
{
    auto geom = smob_get_data<std::shared_ptr<ballistae::camera_priv>*>(obj);
    delete geom;
    return 0;
}

SCM subsmob_mark(SCM obj)
{
    return SCM_BOOL_F;
}

int subsmob_print(SCM obj, SCM port, scm_print_state *pstate)
{
    scm_write_line(scm_from_utf8_string("#<ballistae/camera>"), SCM_UNDEFINED);
    return 0;
}

SCM subsmob_equalp(SCM a, SCM b)
{
    auto a_p = smob_get_data<std::shared_ptr<ballistae::camera_priv>*>(a);
    auto b_p = smob_get_data<std::shared_ptr<ballistae::camera_priv>*>(b);

    return (a_p == b_p) ? SCM_BOOL_T : SCM_BOOL_F;
}

}

}
