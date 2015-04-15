#include <libguile_ballistae/affine_transform.hh>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/affine_transform.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

namespace affine_transform
{

SCM compose(SCM rest_scm)
{
    bl::affine_transform<double, 3> total;

    for(SCM cur = rest_scm; !scm_is_null(cur); cur = scm_cdr(cur))
    {
        SCM aff = ensure_smob(scm_car(cur), flag_affine_transform);
        total = from_scm(aff) * total;
    }

    auto affine_p = new bl::affine_transform<double, 3>(total);
    return new_smob(flag_affine_transform, affine_p);
}

SCM identity()
{
    auto affine_p = new bl::affine_transform<double, 3>();
    return new_smob(flag_affine_transform, affine_p);
}

SCM translation(SCM t_scm)
{
    auto t_p = smob_get_data<arma::Col<double>*>(t_scm);
    bl::fixvec<double, 3> t = *t_p;

    auto affine_p = new bl::affine_transform<double, 3>(
        bl::translation<double, 3>(t)
    );

    scm_remember_upto_here_1(t_scm);

    return new_smob(flag_affine_transform, affine_p);
}

SCM scaling(SCM s_scm)
{
    double s = scm_to_double(s_scm);

    auto affine_p = new bl::affine_transform<double, 3>(
        bl::scaling<double, 3>(s)
    );

    return new_smob(flag_affine_transform, affine_p);
}

SCM rotation(SCM axis_scm, SCM angle_scm)
{
    auto axis_p = smob_get_data<arma::Col<double>*>(axis_scm);
    bl::fixvec<double, 3> axis = *axis_p;
    double angle = scm_to_double(angle_scm);

    auto affine_p = new bl::affine_transform<double, 3>(
        bl::rotation(axis, angle)
    );

    scm_remember_upto_here_1(axis_scm);

    return new_smob(flag_affine_transform, affine_p);
}

SCM basis_mapping(SCM t0_scm, SCM t1_scm, SCM t2_scm)
{
    auto t0_p = smob_get_data<arma::Col<double>*>(t0_scm);
    bl::fixvec<double, 3> t0 = *t0_p;
    auto t1_p = smob_get_data<arma::Col<double>*>(t1_scm);
    bl::fixvec<double, 3> t1 = *t1_p;
    auto t2_p = smob_get_data<arma::Col<double>*>(t2_scm);
    bl::fixvec<double, 3> t2 = *t2_p;

    scm_remember_upto_here_1(t0_scm);
    scm_remember_upto_here_1(t1_scm);
    scm_remember_upto_here_1(t2_scm);

    auto affine_p = new bl::affine_transform<double, 3>(
        bl::basis_mapping<double>(t0, t1, t2)
    );

    return new_smob(flag_affine_transform, affine_p);
}

bl::affine_transform<double, 3> from_scm(SCM t_scm)
{
    ensure_smob(t_scm, flag_affine_transform);
    return *smob_get_data<bl::affine_transform<double, 3>*>(t_scm);
}

size_t subsmob_free(SCM obj)
{
    auto a_p = smob_get_data<bl::affine_transform<double, 3>*>(obj);
    delete a_p;
    return 0;
}

SCM subsmob_mark(SCM obj)
{
    return SCM_BOOL_F;
}

int subsmob_print(SCM obj, SCM port, scm_print_state *pstate)
{
    SCM fmt = scm_from_utf8_string("#<ballistae/affine_transform>");
    scm_simple_format(port, fmt, SCM_EOL);
    return 0;
}

SCM subsmob_equalp(SCM a, SCM b)
{
    auto a_p = smob_get_data<bl::affine_transform<double, 3>*>(a);
    auto b_p = smob_get_data<bl::affine_transform<double, 3>*>(b);

    return (a_p == b_p) ? SCM_BOOL_T : SCM_BOOL_F;
}

subsmob_fns init()
{
    scm_c_define_gsubr("bsta/backend/aff-t/identity",      0, 0, 0, (scm_t_subr) identity);
    scm_c_define_gsubr("bsta/backend/aff-t/translation",   1, 0, 0, (scm_t_subr) translation);
    scm_c_define_gsubr("bsta/backend/aff-t/scaling",       1, 0, 0, (scm_t_subr) scaling);
    scm_c_define_gsubr("bsta/backend/aff-t/rotation",      2, 0, 0, (scm_t_subr) rotation);
    scm_c_define_gsubr("bsta/backend/aff-t/basis-mapping", 3, 0, 0, (scm_t_subr) basis_mapping);
    scm_c_define_gsubr("bsta/backend/aff-t/compose",       0, 0, 1, (scm_t_subr) compose);

    return {&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp};
}

}

}
