#ifndef LIBGUILE_ARMADILLO_HH
#define LIBGUILE_ARMADILLO_HH

#include <iomanip>
#include <sstream>

#include <armadillo>

#include <cstddef> // workaround for bug in GMP
#include <libguile.h>

#include <libguile_armadillo/utility.hh>

namespace arma_guile
{

extern scm_t_bits arma_smob_tag;

////////////////////////////////////////////////////////////////////////////////
/// Predicate: is [obj] an arma smob?
////////////////////////////////////////////////////////////////////////////////
SCM arma_p(SCM obj);

////////////////////////////////////////////////////////////////////////////////
/// Type-based lookup of subsmob flags.
////////////////////////////////////////////////////////////////////////////////
template<class Elem> struct col_info {};
template<> struct col_info<double> {
    static constexpr scm_t_bits flags = 0;
    static constexpr const char *const scm_name = "arma/b64col";
};

////////////////////////////////////////////////////////////////////////////////
/// Create a [col] from [elem_list].
////////////////////////////////////////////////////////////////////////////////
///
/// [elem_list] must contain only scheme reals.
template<class Elem>
SCM generic_list_to_col(SCM elem_list)
{
    const char *const gsubr_name = "arma/list->*col";

    SCM_ASSERT_TYPE(
        scm_is_true(scm_list_p(elem_list)),
        elem_list,
        SCM_ARG1,
        gsubr_name,
        "elem_list must be a list of real numbers."
    );

    SCM cur_head = elem_list;
    while(!scm_is_null(cur_head))
    {
        SCM cur_elem = scm_car(cur_head);
        cur_head = scm_cdr(cur_head);

        SCM_ASSERT_TYPE(
            scm_real_p(cur_elem),
            elem_list,
            SCM_ARG1,
            gsubr_name,
            "elem_list contains a non-real element."
        );
    }

    SCM result = scm_new_smob(
        arma_smob_tag,
        reinterpret_cast<scm_t_bits>(nullptr)
    );

    SCM_SET_SMOB_FLAGS(result, col_info<Elem>::flags);

    // No guile error may be thrown below this point.

    auto *vec_p = new arma::Col<Elem>(
        scm_to_size_t(scm_length(elem_list))
    );

    std::size_t i = 0;
    cur_head = elem_list;
    while(!scm_is_null(cur_head))
    {
        SCM cur_elem = scm_car(cur_head);
        cur_head = scm_cdr(cur_head);

        (*vec_p)(i) = scm_to_double(cur_elem);

        ++i;
    }

    smob_set_data(result, vec_p);

    return result;
}

////////////////////////////////////////////////////////////////////////////////
/// Get an arma::Col back from an appropriate smob.
////////////////////////////////////////////////////////////////////////////////
template<class Elem>
arma::Col<Elem> extract_col(SCM col_scm)
{
    return *smob_get_data<arma::Col<Elem>*>(col_scm);
}

////////////////////////////////////////////////////////////////////////////////
/// Generic [free] function for [arma/col] subsmobs.
////////////////////////////////////////////////////////////////////////////////
template<class Elem>
size_t generic_col_free(SCM col_scm)
{
    auto col_p = smob_get_data<arma::Col<Elem>*>(col_scm);
    delete col_p;
    scm_remember_upto_here_1(col_scm);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Generic [mark] function for [arma/col] subsmobs.
////////////////////////////////////////////////////////////////////////////////
template<class Elem>
SCM generic_col_mark(SCM col_scm)
{
    return SCM_BOOL_F;
}

////////////////////////////////////////////////////////////////////////////////
/// Generic [print] function for [arma/col] subsmobs.
////////////////////////////////////////////////////////////////////////////////
template<class Elem>
int generic_col_print(SCM col_scm, SCM port, scm_print_state *pstate)
{
    auto col_p = smob_get_data<arma::Col<Elem>*>(col_scm);

    {
        std::stringstream os;
        os << "#<" << col_info<Elem>::scm_name  << " (";
        os << col_p->n_elem << " elems) (";
        for(std::size_t i = 0; i < col_p->n_elem; ++i)
        {
            os << (*col_p)(i) << " ";
        }
        os << ")>";

        scm_write_line(scm_from_utf8_string(os.str().c_str()), SCM_UNDEFINED);
    }

    scm_remember_upto_here_1(col_scm);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Generic [equalp] function for arma/col subsmobs.
////////////////////////////////////////////////////////////////////////////////
template<class Elem>
SCM generic_col_equalp(SCM a, SCM b)
{
    auto a_p = smob_get_data<arma::Col<Elem>*>(a);
    auto b_p = smob_get_data<arma::Col<Elem>*>(b);

    return scm_from_bool(arma::all((*a_p) == (*b_p)));
}

////////////////////////////////////////////////////////////////////////////////
/// Predicate: is [obj] an [arma/col] containing [Elem]s?
////////////////////////////////////////////////////////////////////////////////
template<class Elem>
SCM generic_col_p(SCM obj)
{
    return scm_from_bool(
        scm_is_true(arma_p(obj))
        && SCM_SMOB_FLAGS(obj) == col_info<Elem>::flags
    );
}

////////////////////////////////////////////////////////////////////////////////
/// Predicate: is [obj] and [arma/col] of size [dim] containing [Elem]s?
////////////////////////////////////////////////////////////////////////////////
template<class Elem>
SCM generic_col_dim_p(SCM obj, SCM dim)
{
    if(scm_is_true(generic_col_p<Elem>(obj)))
    {
        auto col_p = smob_get_data<arma::Col<Elem>*>(obj);
        return scm_from_bool(col_p->n_elem == scm_to_size_t(dim));
    }
    else
    {
        return SCM_BOOL_F;
    }

}

////////////////////////////////////////////////////////////////////////////////
/// Assertion form of [generic_col_p].
////////////////////////////////////////////////////////////////////////////////
template<class Elem>
SCM generic_assert_col(SCM obj)
{
    SCM_ASSERT_TYPE(
        scm_is_true(generic_col_p<Elem>(obj)),
        obj,
        SCM_ARG1,
        "arma_guile::generic_assert_col",
        "Argument 1 is not an arma/col of correct element type."
    );
    return SCM_BOOL_T;
}

////////////////////////////////////////////////////////////////////////////////
/// Assertion form of [generic_col_dim_p].
////////////////////////////////////////////////////////////////////////////////
template<class Elem>
SCM generic_assert_col_dim(SCM obj, SCM dim)
{
    SCM_ASSERT_TYPE(
        scm_is_true(generic_col_dim_p<Elem>(obj, dim)),
        obj,
        SCM_ARG1,
        "arma_guile::generic_assert_col_dim",
        "Argument 1 is not an arma/col of correct element type and dimension."
    );
    return SCM_BOOL_T;
}

}

extern "C" void libguile_armadillo_init()
    __attribute__((visibility("default")));

#endif
