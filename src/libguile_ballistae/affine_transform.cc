#include <libguile_ballistae/affine_transform.hh>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

#include <libballistae/affine_transform.hh>

#include <libguile_ballistae/utility.hh>

namespace ballistae_guile
{

scm_t_bits affine_transform_subsmob_flags;

namespace affine_transform
{

namespace bl = ballistae;

SCM compose(SCM rest_scm)
{
    bl::affine_transform<double, 3> total;

    //rest_scm = scm_reverse(rest_scm);
    for(SCM cur = rest_scm; !scm_is_null(cur); cur = scm_cdr(cur))
    {
        auto cur_p = smob_get_data<bl::affine_transform<double, 3>*>(scm_car(cur));
        total = (*cur_p) * total;
        scm_remember_upto_here_1(cur);
    }

    auto affine_p = new bl::affine_transform<double, 3>(total);

    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(affine_p));
    SCM_SET_SMOB_FLAGS(result, affine_transform_subsmob_flags);
    return result;
}

SCM identity()
{
    auto affine_p = new bl::affine_transform<double, 3>();
    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(affine_p));
    SCM_SET_SMOB_FLAGS(result, affine_transform_subsmob_flags);
    return result;
}

SCM translation(SCM t_scm)
{
    auto t_p = smob_get_data<arma::Col<double>*>(t_scm);
    bl::fixvec<double, 3> t = *t_p;

    auto affine_p = new bl::affine_transform<double, 3>(
        bl::translation<double, 3>(t)
    );

    scm_remember_upto_here_1(t_scm);

    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(affine_p));
    SCM_SET_SMOB_FLAGS(result, affine_transform_subsmob_flags);
    return result;
}

SCM scaling(SCM s_scm)
{
    double s = scm_to_double(s_scm);

    auto affine_p = new bl::affine_transform<double, 3>(
        bl::scaling<double, 3>(s)
    );

    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(affine_p));
    SCM_SET_SMOB_FLAGS(result, affine_transform_subsmob_flags);
    return result;
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

    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(affine_p));
    SCM_SET_SMOB_FLAGS(result, affine_transform_subsmob_flags);
    return result;
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

    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(affine_p));
    SCM_SET_SMOB_FLAGS(result, affine_transform_subsmob_flags);
    return result;
}

SCM affine_transform_p(SCM obj)
{
    return scm_from_bool(
        scm_is_true(ballistae_p(obj))
        && SCM_SMOB_FLAGS(obj) == affine_transform_subsmob_flags
    );
}

SCM ensure_type(SCM obj)
{
    scm_assert_smob_type(smob_tag, obj);
    if(SCM_SMOB_FLAGS(obj) != affine_transform_subsmob_flags)
        scm_wrong_type_arg(nullptr, SCM_ARG1, obj);

    return obj;
}

bl::affine_transform<double, 3> from_scm(SCM t_scm)
{
    ensure_type(t_scm);
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

void init(std::vector<subsmob_fns> &ss_dispatch)
{
    affine_transform_subsmob_flags = ss_dispatch.size();

    scm_c_define_gsubr("bsta/backend/aff-t/identity",      0, 0, 0, (scm_t_subr) identity);
    scm_c_define_gsubr("bsta/backend/aff-t/translation",   1, 0, 0, (scm_t_subr) translation);
    scm_c_define_gsubr("bsta/backend/aff-t/scaling",       1, 0, 0, (scm_t_subr) scaling);
    scm_c_define_gsubr("bsta/backend/aff-t/rotation",      2, 0, 0, (scm_t_subr) rotation);
    scm_c_define_gsubr("bsta/backend/aff-t/basis-mapping", 3, 0, 0, (scm_t_subr) basis_mapping);
    scm_c_define_gsubr("bsta/backend/aff-t/compose",       0, 0, 1, (scm_t_subr) compose);

    ss_dispatch.push_back({&subsmob_free, &subsmob_mark, &subsmob_print, &subsmob_equalp});
}

}

}
