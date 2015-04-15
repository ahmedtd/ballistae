#include <libguile_ballistae/illuminator.hh>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/illuminator.hh>

#include <libguile_ballistae/dense_signal.hh>
#include <libguile_ballistae/illuminator_plugin_interface.hh>
#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

namespace illuminator
{

SCM make_dir(SCM config_alist)
{
    SCM sym_spectrum = scm_from_utf8_symbol("spectrum");
    SCM sym_direction = scm_from_utf8_symbol("direction");

    SCM lu_spectrum = scm_assq_ref(config_alist, sym_spectrum);
    SCM lu_direction = scm_assq_ref(config_alist, sym_direction);

    auto result = new bl::dir_illuminator();

    result->spectrum = dense_signal::from_scm(lu_spectrum);
    result->direction = *smob_get_data<arma::Col<double>*>(lu_direction);

    result->direction = arma::normalise(result->direction);
    
    return new_smob(flag_illuminator, result);
}

SCM make_point_isotropic(SCM config_alist)
{
    SCM sym_spectrum = scm_from_utf8_symbol("spectrum");
    SCM sym_position = scm_from_utf8_symbol("position");

    SCM lu_spectrum = scm_assq_ref(config_alist, sym_spectrum);
    SCM lu_position = scm_assq_ref(config_alist, sym_position);

    auto result = new bl::point_isotropic_illuminator();

    result->spectrum = dense_signal::from_scm(lu_spectrum);
    result->position = *smob_get_data<arma::Col<double>*>(lu_position);

    return new_smob(flag_illuminator, result);
}

SCM make(SCM name, SCM config_alist)
{
    if(scm_is_true(scm_equal_p(name, scm_from_utf8_string("dir"))))
        return make_dir(config_alist);

    if(scm_is_true(scm_equal_p(name, scm_from_utf8_string("point-isotropic"))))
        return make_point_isotropic(config_alist);

    // Otherwise, load from plugin.
    
    SCM plug_soname = scm_string_append(
        scm_list_2(
            scm_from_utf8_string("ballistae_illuminator_"),
            name
        )
    );

    SCM so_handle = scm_dynamic_link(plug_soname);

    auto create_fn = reinterpret_cast<create_illuminator_t>(
        scm_to_pointer(
            scm_dynamic_pointer(
                scm_from_utf8_string("guile_ballistae_illuminator"),
                so_handle
            )
        )
    );

    return new_smob(flag_illuminator, create_fn(config_alist));
}

bl::illuminator* p_from_scm(SCM geom)
{
    ensure_smob(geom, flag_illuminator);
    return smob_get_data<bl::illuminator*>(geom);
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
    SCM fmt = scm_from_utf8_string("#<bsta/illuminator>");
    scm_simple_format(port, fmt, SCM_EOL);
    return 0;
}

SCM subsmob_equalp(SCM a, SCM b)
{
    return (p_from_scm(a) == p_from_scm(b)) ? SCM_BOOL_T : SCM_BOOL_F;
}

subsmob_fns init()
{
    scm_c_define_gsubr("bsta/backend/illum/make", 2, 0, 0, (scm_t_subr) make);

    return {&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp};
}

}

}
