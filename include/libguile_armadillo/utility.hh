#ifndef LIBGUILE_ARMADILLO_UTILITY_HH
#define LIBGUILE_ARMADILLO_UTILITY_HH

// Workaround for bug in GMP, which guile includes.
#include <cstddef>
#include <libguile.h>

#include <armadillo>

namespace arma_guile
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

static constexpr auto smob_get_dcol = smob_get_data<arma::Col<double>*>;

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
