#include <libguile_ballistae/geometry.hh>

#include <memory>

#include <libguile_ballistae/libguile_ballistae.hh>
#include <libguile_ballistae/geometry_plugin_interface.hh>

namespace ballistae_guile
{

namespace geometry
{

SCM make_backend(SCM plug_soname, SCM config_alist)
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

bl::geometry* p_from_scm(SCM geom)
{
    ensure_smob(geom, flag_geometry);
    return smob_get_data<bl::geometry*>(geom);
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
    SCM fmt = scm_from_utf8_string("#<bsta/geometry>");
    scm_simple_format(port, fmt, SCM_EOL);
    return 0;
}

SCM subsmob_equalp(SCM a, SCM b)
{
    return (p_from_scm(a) == p_from_scm(b)) ? SCM_BOOL_T : SCM_BOOL_F;
}

subsmob_fns init()
{
    scm_c_define_gsubr("bsta/backend/geom/make", 2, 0, 0, (scm_t_subr) make_backend);
    
    return {&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp};
}

}

}
