#include <libguile_ballistae/material.hh>

#include <memory>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libguile_ballistae/libguile_ballistae.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

namespace ballistae_guile
{

namespace material
{

SCM make(SCM plug_name, SCM config_alist)
{
    SCM plug_soname = scm_string_append(
        scm_list_2(
            scm_from_utf8_string("ballistae_material_"),
            plug_name
        )
    );

    SCM so_handle = scm_dynamic_link(plug_soname);

    auto create_fn = reinterpret_cast<create_material_t>(
        scm_to_pointer(
            scm_dynamic_pointer(
                scm_from_utf8_string("guile_ballistae_material"),
                so_handle
            )
        )
    );

    return new_smob(flag_material, create_fn(config_alist));
}

bl::material* p_from_scm(SCM matr)
{
    ensure_smob(matr, flag_material);
    return smob_get_data<bl::material*>(matr);
}

size_t subsmob_free(SCM obj)
{
    if(! is_registered(obj))
        delete p_from_scm(obj);
    return 0;
}

SCM subsmob_mark(SCM obj)
{
    return SCM_BOOL_F;
}

int subsmob_print(SCM obj, SCM port, scm_print_state *pstate)
{
    scm_write_line(scm_from_utf8_string("#<bsta/material>"), SCM_UNDEFINED);
    return 0;
}

SCM subsmob_equalp(SCM a, SCM b)
{
    return (p_from_scm(a) == p_from_scm(b)) ? SCM_BOOL_T : SCM_BOOL_F;
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize the [matr] subsmob/subsystem.
////////////////////////////////////////////////////////////////////////////////
subsmob_fns init()
{
    scm_c_define_gsubr("bsta/backend/matr/make", 2, 0, 0, (scm_t_subr) make);

    return {&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp};
}

}

}
