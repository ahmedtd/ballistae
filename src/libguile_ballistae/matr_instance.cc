#include <libguile_ballistae/matr_instance.hh>

#include <memory>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libguile_ballistae/matr_plugin_interface.hh>
#include <libguile_ballistae/utility.hh>

namespace ballistae_guile
{

////////////////////////////////////////////////////////////////////////////////
/// Allocated storage for the [matr] subsmob flags.
////////////////////////////////////////////////////////////////////////////////
scm_t_bits matr_subsmob_flags;

namespace matr_instance
{

////////////////////////////////////////////////////////////////////////////////
/// Initialize the [matr] subsmob/subsystem.
////////////////////////////////////////////////////////////////////////////////
void init(std::vector<subsmob_fns> &ss_dispatch)
{
    matr_subsmob_flags = ss_dispatch.size();

    scm_c_define_gsubr("ballistae/matr/make", 2, 0, 0, (scm_t_subr) make);
    scm_c_define_gsubr("ballistae/matr?", 1, 0, 0, (scm_t_subr) matr_p);

    scm_c_export(
        "ballistae/matr/make",
        "ballistae/matr?",
        nullptr
    );

    ss_dispatch.push_back({&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp});
}

SCM make(SCM plug_name, SCM config_alist)
{
    namespace b = ballistae;

    SCM plug_soname = scm_string_append(
        scm_list_2(
            scm_from_utf8_string("ballistae_matr_plugin_"),
            plug_name
        )
    );

    SCM so_handle = scm_dynamic_link(plug_soname);

    auto create_fn = reinterpret_cast<matrplug_create_from_alist_t>(
        scm_to_pointer(
            scm_dynamic_pointer(
                scm_from_utf8_string("ballistae_matr_create_from_alist"),
                so_handle
            )
        )
    );

    // Allocate a new shared_ptr on the heap and copy-initialize it
    auto matr_p = new std::shared_ptr<ballistae::matr_priv>(
        create_fn(config_alist)
    );

    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(matr_p));
    SCM_SET_SMOB_FLAGS(result, matr_subsmob_flags);
    return result;
}

SCM matr_p(SCM obj)
{
    return scm_from_bool(
        scm_is_true(ballistae_p(obj))
        && SCM_SMOB_FLAGS(obj) == matr_subsmob_flags
    );
}

size_t subsmob_free(SCM obj)
{
    auto geom = smob_get_data<std::shared_ptr<ballistae::matr_priv>*>(obj);
    delete geom;
    return 0;
}

SCM subsmob_mark(SCM obj)
{
    return SCM_BOOL_F;
}

int subsmob_print(SCM obj, SCM port, scm_print_state *pstate)
{
    scm_write_line(scm_from_utf8_string("#<ballistae/matr>"), SCM_UNDEFINED);
    return 0;
}

SCM subsmob_equalp(SCM a, SCM b)
{
    auto a_p = smob_get_data<std::shared_ptr<ballistae::matr_priv>*>(a);
    auto b_p = smob_get_data<std::shared_ptr<ballistae::matr_priv>*>(b);

    return (a_p == b_p) ? SCM_BOOL_T : SCM_BOOL_F;
}

}

}
