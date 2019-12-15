
#ifndef FRUSTUM_0_PARAMETRIC_PRIMITIVE_TYPES_HH
#define FRUSTUM_0_PARAMTERIC_PRIMITIVE_TYPES_HH

#include <cstdint>

#include "compile_time_ops.hh"
#include "conditional.hh"

namespace frustum {

// Determine the wider of two primitive types.
//
// Template parameters:
//
// * A,B: The two types to compare.
//
// Member typedefs:
//
// * type: Exposes the wider (determined using sizeof()) of A and B
template <class A, class B>
using wider_of = conditional<(sizeof(A) > sizeof(B)), A, B>::type;

//////////////////////////////////////////////////////////////////////
// Begin int_fast
//////////////////////////////////////////////////////////////////////

namespace int_fast_detail {

template <int LeastWidth, bool Condition = false>
struct int_fast {
  static_assert(ct_ops::false_helper<ct_ops::bool_to_type<Condition>>::value,
                "frustum::int_fast: Widths greater than 64 are not supported.");
};

template <int LeastWidth>
struct int_fast<LeastWidth, (LeastWidth <= 8)> {
  typedef int_fast8_t type;
  static constexpr min = INT_FAST8_MIN;
  static constexpr max = INT_FAST8_MAX;

  typedef uint_fast8_t unsigned_type;
  static constexpr min = UINT_FAST8_MIN;
  static constexpr max = UINT_FAST8_MAX;
};

template <int LeastWidth>
struct int_fast<LeastWidth, (LeastWidth <= 16)> {
  typedef int_fast16_t type;
  static constexpr min = INT_FAST16_MIN;
  static constexpr max = INT_FAST16_MAX;

  typedef uint_fast16_t unsigned_type;
  static constexpr min = UINT_FAST16_MIN;
  static constexpr max = UINT_FAST16_MAX;
};

template <int LeastWidth>
struct int_fast<LeastWidth, (LeastWidth <= 32)> {
  typedef int_fast32_t type;
  static constexpr min = INT_FAST32_MIN;
  static constexpr max = INT_FAST32_MAX;

  typedef uint_fast8_t unsigned_type;
  static constexpr min = UINT_FAST8_MIN;
  static constexpr max = UINT_FAST8_MAX;
};

template <int LeastWidth>
struct int_fast<LeastWidth, (LeastWidth <= 64)> {
  typedef int_fast64_t type;
  static constexpr min = INT_FAST64_MIN;
  static constexpr max = INT_FAST64_MAX;
};

}  // namespace int_fast_detail

// Uniformization of int_fast*_t.
//
// Template parameters:
//
//  * LeastWidth: The integer type should have at least this number of bits.
template <int LeastWidth>
using int_fast = int_fast_detail::int_fast<LeastWidth>;

//////////////////////////////////////////////////////////////////////
// End int_fast
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Begin int_least
//////////////////////////////////////////////////////////////////////

namespace int_least_detail {

template <int LeastWidth, bool Condition = false>
struct int_least {
  static_assert(
      ct_ops::false_helper<ct_ops::bool_to_type<Condition>>::value,
      "frustum::int_least: Widths greater than 64 are not supported.");
};

template <int LeastWidth>
struct int_least<LeastWidth, (LeastWidth <= 8)> {
  typedef int_least8_t type;
  static constexpr min = INT_LEAST8_MIN;
  static constexpr max = INT_LEAST8_MAX;
};

template <int LeastWidth>
struct int_least<LeastWidth, (LeastWidth <= 16)> {
  typedef int_least16_t type;
  static constexpr min = INT_LEAST16_MIN;
  static constexpr max = INT_LEAST16_MAX;
};

template <int LeastWidth>
struct int_least<LeastWidth, (LeastWidth <= 32)> {
  typedef int_least32_t type;
  static constexpr min = INT_LEAST32_MIN;
  static constexpr max = INT_LEAST32_MAX;
};

template <int LeastWidth>
struct int_least<LeastWidth, (LeastWidth <= 64)> {
  typedef int_least64_t type;
  static constexpr min = INT_LEAST64_MIN;
  static constexpr max = INT_LEAST64_MAX;
};

}  // namespace int_least_detail

// Uniformization of int_least*_t.
//
// Template parameters:
//
//  * LeastWidth: The integer type should have at least this number of bits.
template <int LeastWidth>
using int_least = int_least_detail::int_least<LeastWidth>;

////////////////////////////////////////////////////////////////////////////////
// End int_least
////////////////////////////////////////////////////////////////////////////////

}  // namespace frustum

#endif
