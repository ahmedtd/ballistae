#ifndef LIBBALLISTAE_GUILE_LIBBALLISTAE_GUILE_HH
#define LIBBALLISTAE_GUILE_LIBBALLISTAE_GUILE_HH

#include <vector>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

////////////////////////////////////////////////////////////////////////////////
/// Entry point for the library.
////////////////////////////////////////////////////////////////////////////////
///
/// The library should be loaded from the guile interpreter using the form
///
///     (load-extension "libballistae_guile" "libballistae_guile_init")
extern "C" void libguile_ballistae_init() __attribute__((visibility("default")));

namespace ballistae_guile
{

////////////////////////////////////////////////////////////////////////////////
/// Is [obj] a ballistae smob?
////////////////////////////////////////////////////////////////////////////////
SCM ballistae_p(SCM obj);

using smob_free_t = size_t (*)(SCM);
using smob_mark_t = SCM (*)(SCM);
using smob_print_t = int (*)(SCM, SCM, scm_print_state*);
using smob_equalp_t = SCM (*)(SCM, SCM);

////////////////////////////////////////////////////////////////////////////////
/// The smob tag.
////////////////////////////////////////////////////////////////////////////////
///
/// Guile assigns each new smob type an 8-bit tag.  Since there are so few
/// available tags, it's best to use one smob for all the object types an
/// extension will define.  Sub-types can be identified using the smob's 16-bit
/// flag field, accessed using SCM_SMOB_FLAGS.
extern scm_t_bits smob_tag;

////////////////////////////////////////////////////////////////////////////////
/// The required functions for a subsmob.
////////////////////////////////////////////////////////////////////////////////
struct subsmob_fns
{
    smob_free_t   free_fn;
    smob_mark_t   mark_fn;
    smob_print_t  print_fn;
    smob_equalp_t equalp_fn;
};

////////////////////////////////////////////////////////////////////////////////
/// The signature for each subsystem's initialization function.
////////////////////////////////////////////////////////////////////////////////
///
/// In the init function, each subsystem should set up subsmob types
/// (registering them into the provided table), register guile subroutines, etc.
using init_t = void (*)(std::vector<subsmob_fns>&);

////////////////////////////////////////////////////////////////////////////////
/// Tag dispatch table for smob subtypes.
////////////////////////////////////////////////////////////////////////////////
///
/// If guile sends us a ballistae smob, we dispatch it to the appropriate
/// subsmob functions by treating the flag bits as an index into this table.
extern std::vector<subsmob_fns> subsmob_dispatch_table;

}

#endif
