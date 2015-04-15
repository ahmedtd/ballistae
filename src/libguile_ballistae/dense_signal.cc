#include <libguile_ballistae/dense_signal.hh>

#include <vector>

#include <libballistae/dense_signal.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

namespace dense_signal
{

SCM from_list(SCM lo, SCM hi, SCM val_list)
{
    auto p = new bl::dense_signal<double>();
    p->src_val = scm_to_double(lo);
    p->lim_val = scm_to_double(hi);

    for(; !scm_is_null(val_list); val_list = scm_cdr(val_list))
    {
        p->samples.push_back(scm_to_double(scm_car(val_list)));
    }

    return new_smob(flag_dense_signal, p);
}

SCM pulse(SCM pulse_src, SCM pulse_lim, SCM pulse_power)
{
    auto p = new bl::dense_signal<double>(
        bl::pulse<double>(
            390,
            835,
            89,
            scm_to_double(pulse_src),
            scm_to_double(pulse_lim),
            scm_to_double(pulse_power)
        )
    );

    return new_smob(flag_dense_signal, p);
}

SCM red(SCM intensity)
{
    auto p = new bl::dense_signal<double>(scm_to_double(intensity) * bl::red<double>());
    return new_smob(flag_dense_signal, p);
}

SCM green(SCM intensity)
{
    auto p = new bl::dense_signal<double>(scm_to_double(intensity) * bl::green<double>());
    return new_smob(flag_dense_signal, p);
}

SCM blue(SCM intensity)
{
    auto p = new bl::dense_signal<double>(scm_to_double(intensity) * bl::blue<double>());
    return new_smob(flag_dense_signal, p);
}

SCM rgb_to_spectral(SCM red, SCM green, SCM blue)
{
    auto sig = scm_to_double(red) * bl::red<double>()
        + scm_to_double(green) * bl::green<double>()
        + scm_to_double(blue) * bl::blue<double>();

    return new_smob(flag_dense_signal, new bl::dense_signal<double>(std::move(sig)));
}

bl::dense_signal<double> from_scm(SCM obj)
{
    ensure_smob(obj, flag_dense_signal);
    return *smob_get_data<bl::dense_signal<double>*>(obj);
}

size_t subsmob_free(SCM obj)
{
    auto p = smob_get_data<bl::dense_signal<double>*>(obj);
    delete p;
    return 0;
}

SCM subsmob_mark(SCM obj)
{
    return SCM_BOOL_F;
}

int subsmob_print(SCM obj, SCM port, scm_print_state *pstate)
{
    SCM fmt = scm_from_utf8_string("#<ballistae/dense_function>");
    scm_simple_format(port, fmt, SCM_EOL);
    return 0;
}

SCM subsmob_equalp(SCM a, SCM b)
{
    return (from_scm(a) == from_scm(b)) ? SCM_BOOL_T : SCM_BOOL_F;
}

subsmob_fns init()
{
    scm_c_define_gsubr("bsta/backend/dense-signal/from-list", 3, 0, 0, (scm_t_subr) from_list);
    scm_c_define_gsubr("bsta/backend/dense-signal/pulse", 3, 0, 0, (scm_t_subr) pulse);

    scm_c_define_gsubr("bsta/backend/dense-signal/red", 1, 0, 0, (scm_t_subr) red);
    scm_c_define_gsubr("bsta/backend/dense-signal/green", 1, 0, 0, (scm_t_subr) green);
    scm_c_define_gsubr("bsta/backend/dense-signal/blue", 1, 0, 0, (scm_t_subr) blue);

    scm_c_define_gsubr("bsta/backend/dense-signal/rgb-to-spectral", 3, 0, 0, (scm_t_subr) rgb_to_spectral);

    return {&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp};
}

}

}
