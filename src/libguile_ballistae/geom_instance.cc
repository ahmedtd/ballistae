#include <libguile_ballistae/geom_instance.hh>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libguile_ballistae/geom_plugin_interface.hh>
#include <libguile_ballistae/utility.hh>

namespace ballistae_guile
{

scm_t_bits geom_instance_subsmob_flags;

namespace geom_instance
{

////////////////////////////////////////////////////////////////////////////////
/// Init procedure for geom_instance subsmob.
////////////////////////////////////////////////////////////////////////////////
void init(std::vector<subsmob_fns> &ss_dispatch)
{
    geom_instance_subsmob_flags = ss_dispatch.size();

    scm_c_define_gsubr("ballistae/geom/make", 2, 0, 0, (scm_t_subr) make);
    scm_c_define_gsubr("ballistae/geom?", 1, 0, 0, (scm_t_subr) geom_p);
    
    scm_c_export(
        "ballistae/geom/make",
        "ballistae/geom?",
        nullptr
    );
    
    ss_dispatch.push_back({&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp});
}

////////////////////////////////////////////////////////////////////////////////
/// Create a geometry instance.
////////////////////////////////////////////////////////////////////////////////
SCM make(SCM plug_soname, SCM config_alist)
{
    namespace b = ballistae;

    plug_soname = scm_string_append(
        scm_list_2(
            scm_from_utf8_string("ballistae_geom_plugin_"),
            plug_soname
        )
    );
    
    SCM so_handle = scm_dynamic_link(plug_soname);

    auto create_fn = reinterpret_cast<geomplug_create_from_alist_t>(
        scm_to_pointer(
            scm_dynamic_pointer(
                scm_from_utf8_string("ballistae_geom_create_from_alist"),
                so_handle
            )
        )
    );

    auto geom_p = new std::shared_ptr<ballistae::geom_priv>(
        create_fn(config_alist)
    );
    
    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(geom_p));
    SCM_SET_SMOB_FLAGS(result, geom_instance_subsmob_flags);
    return result;
}

SCM geom_p(SCM obj)
{
    return scm_from_bool(
        scm_is_true(ballistae_p(obj))
        && SCM_SMOB_FLAGS(obj) == geom_instance_subsmob_flags
    );
}

SCM assert_type(SCM obj)
{    
    scm_assert_smob_type(smob_tag, obj);
    if(SCM_SMOB_FLAGS(obj) != geom_instance_subsmob_flags)
    { 
        scm_wrong_type_arg(
            "libguile_ballistae::geom_instance::assert_type",
            SCM_ARG1,
            obj
        );
    }

    return SCM_BOOL_T;
}

size_t subsmob_free(SCM obj)
{
    auto geom = smob_get_data<std::shared_ptr<ballistae::geom_priv>*>(obj);
    delete geom;
    return 0;
}

SCM subsmob_mark(SCM obj)
{
    return SCM_BOOL_F;
}

int subsmob_print(SCM obj, SCM port, scm_print_state *pstate)
{
    SCM fmt = scm_from_utf8_string("#<ballistae/geom>");
    scm_simple_format(port, fmt, SCM_EOL);
    return 0;
}

SCM subsmob_equalp(SCM a, SCM b)
{
    auto a_p = smob_get_data<std::shared_ptr<ballistae::geom_priv>*>(a);
    auto b_p = smob_get_data<std::shared_ptr<ballistae::geom_priv>*>(b);

    return (a_p == b_p) ? SCM_BOOL_T : SCM_BOOL_F;
}

}

}