#ifndef LIBBALLISTAE_GUILE_LIBBALLISTAE_GUILE_HH
#define LIBBALLISTAE_GUILE_LIBBALLISTAE_GUILE_HH

#include <vector>

#include <cstddef> // workaround for bug in GMP.
#include <libguile.h>

////////////////////////////////////////////////////////////////////////////////
/// Entry point for the library.
////////////////////////////////////////////////////////////////////////////////
///
/// The library has a corresponding scheme file that should be used for loading.
extern "C" void libguile_ballistae_init() __attribute__((visibility("default")));

namespace ballistae_guile
{

namespace bl = ballistae;

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
/// Subsmob tags.
////////////////////////////////////////////////////////////////////////////////
///
/// Values used in the 16-bit flag field.
constexpr scm_t_bits flag_scene            = 0;
constexpr scm_t_bits flag_camera           = 1;
constexpr scm_t_bits flag_geometry         = 2;
constexpr scm_t_bits flag_material         = 3;
constexpr scm_t_bits flag_illuminator      = 4;
constexpr scm_t_bits flag_affine_transform = 5;
constexpr scm_t_bits flag_dense_signal     = 6;

////////////////////////////////////////////////////////////////////////////////
/// Manipulation of subsmob types.
////////////////////////////////////////////////////////////////////////////////
///
/// The MSB of the flags field is used to track whether or not the subsmob has
/// been registered with a scene.  If it is, then the scene controls the
/// lifetime of the allocated object, not the smob.

constexpr scm_t_bits mask_subsmob_type = 0x0fff;
constexpr scm_t_bits mask_registered = 0x8000;

bool is_registered(SCM obj);
SCM set_registered(SCM obj, bool status);

SCM set_subsmob_type(SCM obj, scm_t_bits type);
scm_t_bits get_subsmob_type(SCM obj);

////////////////////////////////////////////////////////////////////////////////
/// Create a ballistae smob with given flags and data.
////////////////////////////////////////////////////////////////////////////////
template<class Data>
SCM new_smob(scm_t_bits flag, Data data)
{
    SCM result = scm_new_smob(smob_tag, reinterpret_cast<scm_t_bits>(data));
    SCM_SET_SMOB_FLAGS(result, flag);
    return result;
}

SCM ensure_smob(SCM obj, scm_t_bits flag);

template<typename T>
T smob_get_data(SCM obj)
{
    static_assert(
        sizeof(T) == sizeof(scm_t_bits),
        "Type to retrieve must be the same size as scm_t_bits."
    );

    return reinterpret_cast<T>(SCM_SMOB_DATA(obj));
}

template<typename T>
SCM smob_set_data(SCM obj, T data)
{
    static_assert(
        sizeof(T) == sizeof(scm_t_bits),
        "Type to store must be the same size as scm_t_bits."
    );

    return SCM_SET_SMOB_DATA(obj, reinterpret_cast<scm_t_bits>(data));
}

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
