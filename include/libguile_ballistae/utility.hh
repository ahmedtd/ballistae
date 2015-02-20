#ifndef LIBBALLISTAE_GUILE_UTILITY_HH
#define LIBBALLISTAE_GUILE_UTILITY_HH

// Workaround for bug in GMP, which guile includes.
#include <cstddef>
#include <libguile.h>

namespace ballistae_guile
{

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

}

#endif
