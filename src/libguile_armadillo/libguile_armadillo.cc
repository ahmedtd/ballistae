#include <libguile_armadillo/libguile_armadillo.hh>

#include <cstddef> // workaround for bug in GMP
#include <libguile.h>

#include <armadillo>

namespace arma_guile
{

scm_t_bits arma_smob_tag;

////////////////////////////////////////////////////////////////////////////////
/// Instantiations of templated subsmob functions
////////////////////////////////////////////////////////////////////////////////

template size_t generic_col_free<double>(SCM col_scm);

// ...

////////////////////////////////////////////////////////////////////////////////
/// The required functions for a subsmob.
////////////////////////////////////////////////////////////////////////////////

using smob_free_t = size_t (*)(SCM);
using smob_mark_t = SCM (*)(SCM);
using smob_print_t = int (*)(SCM, SCM, scm_print_state*);
using smob_equalp_t = SCM (*)(SCM, SCM);

struct subsmob_fns
{
    smob_free_t   free_fn;
    smob_mark_t   mark_fn;
    smob_print_t  print_fn;
    smob_equalp_t equalp_fn;
};

////////////////////////////////////////////////////////////////////////////////
/// [arma] smob dispatch table
////////////////////////////////////////////////////////////////////////////////
///
/// The arma smob free, mark, print, and equalp functions use this table
/// (indexed by the smob's flag field), to dispatch to the appropriate
/// implementation.
#define GUILE_ARMA_TYPE_TO_FNS(elem_type) {           \
        &generic_col_free<elem_type>,                 \
            &generic_col_mark<elem_type>,             \
            &generic_col_print<elem_type>,            \
            &generic_col_equalp<elem_type>            \
}

std::vector<subsmob_fns> dispatch_table = {
    GUILE_ARMA_TYPE_TO_FNS(double)
};
#undef GUILE_ARMA_TYPE_TO_FNS

////////////////////////////////////////////////////////////////////////////////
/// Top-level [free] for arma smobs.
////////////////////////////////////////////////////////////////////////////////
size_t smob_free(SCM obj)
{
    scm_t_bits flags = SCM_SMOB_FLAGS(obj);
    smob_free_t free_fn = dispatch_table[flags].free_fn;
    return (*free_fn)(obj);
}

////////////////////////////////////////////////////////////////////////////////
/// Top-level [mark] for arma smobs.
////////////////////////////////////////////////////////////////////////////////
SCM smob_mark(SCM obj)
{
    scm_t_bits flags = SCM_SMOB_FLAGS(obj);
    smob_mark_t mark_fn = dispatch_table[flags].mark_fn;
    return (*mark_fn)(obj);
}

////////////////////////////////////////////////////////////////////////////////
/// Top-level [print] for arma smobs.
////////////////////////////////////////////////////////////////////////////////
int smob_print(SCM obj, SCM port, scm_print_state *pstate)
{
    scm_t_bits flags = SCM_SMOB_FLAGS(obj);
    smob_print_t print_fn = dispatch_table[flags].print_fn;
    return (*print_fn)(obj, port, pstate);
}

////////////////////////////////////////////////////////////////////////////////
/// Top-level [equalp] for arma smobs.
////////////////////////////////////////////////////////////////////////////////
SCM smob_equalp(SCM a, SCM b)
{
    scm_t_bits flags_a = SCM_SMOB_FLAGS(a);
    scm_t_bits flags_b = SCM_SMOB_FLAGS(b);

    if(flags_a != flags_b)
    {
        return SCM_BOOL_F;
    }
    else
    {
        smob_equalp_t equalp_fn = dispatch_table[flags_a].equalp_fn;
        return (*equalp_fn)(a, b);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Predicate: is [obj] an arma smob?
////////////////////////////////////////////////////////////////////////////////
SCM arma_p(SCM obj)
{
    return scm_from_bool(SCM_SMOB_PREDICATE(arma_smob_tag, obj));
}

}

extern "C" void libguile_armadillo_init()
{
    namespace ag = arma_guile;

    ag::arma_smob_tag = scm_make_smob_type("arma", 0);
    scm_set_smob_free(ag::arma_smob_tag, &ag::smob_free);
    scm_set_smob_mark(ag::arma_smob_tag, &ag::smob_mark);
    scm_set_smob_print(ag::arma_smob_tag, &ag::smob_print);
    scm_set_smob_equalp(ag::arma_smob_tag, &ag::smob_equalp);

    scm_c_define_gsubr("arma/dvec", 1, 0, 0, (scm_t_subr) &ag::generic_list_to_col<double>);
    scm_c_export("arma/dvec", nullptr);

    scm_c_define_gsubr("arma/b64col?", 1, 0, 0, (scm_t_subr) &ag::generic_col_p<double>);
    scm_c_export("arma/b64col?", nullptr);

    scm_c_define_gsubr("arma/assert-b64col", 1, 0, 0, (scm_t_subr) &ag::generic_assert_col<double>);
    scm_c_export("arma/assert-b64col", nullptr);

    scm_c_define_gsubr("arma/b64col-dim-?", 2, 0, 0, (scm_t_subr) &ag::generic_col_dim_p<double>);
    scm_c_export("arma/b64col-dim-?", nullptr);

    scm_c_define_gsubr("arma/assert-b64col-dim", 2, 0, 0, (scm_t_subr) &ag::generic_assert_col_dim<double>);
    scm_c_export("arma/assert-b64col-dim", nullptr);

    scm_c_define_gsubr("arma?", 1, 0, 0, (scm_t_subr) &ag::arma_p);
    scm_c_export("arma?", nullptr);
}
