
#ifndef FRUSTUM_0_CONDITIONAL_HH
#define FRUSTUM_0_CONDITIONAL_HH

namespace frustum {

//////////////////////////////////////////////////////////////////////
// Begin conditional
//////////////////////////////////////////////////////////////////////

// Stand-in for std::conditional, which isn't yet available (along with the rest
// of the C++ standard library) on AVR.  If Condition is true, TrueType is
// exposed through the internal typedef, and vice-versa.
//
// Template parameters:
//
//  * Condition: A compile-time constant boolean condition.
//
//  * TrueType: Type to be exposed in the internal "type" typedef if Condition
//  * is true.
//
//  * FalseType: Type to be exposed in the internal "type" typedef if Condition
//  * is false.
template <bool Condition, class TrueType, class FalseType>
struct conditional {
  typedef TrueType type;
};

template <class TrueType, class FalseType>
struct conditional<false, TrueType, FalseType> {
  typedef FalseType type;
};

//////////////////////////////////////////////////////////////////////
// End conditional
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Begin conditional_int
//////////////////////////////////////////////////////////////////////

// Analogy of conditional for compile-time constant integers.
//
// Template parameters:
//
// * Condition: A compile-time constant boolean condition.
//
// * TrueVal, FalseVal: Values that are exposed if condition is true or false,
// * respectively.
template <bool Condition, int TrueVal, int FalseVal>
struct conditional_int {
  static constexpr int value = TrueVal;
};

template <int TrueVal, int FalseVal>
struct conditional_int<false, TrueVal, FalseVal> {
  static constexpr int value = FalseVal;
};

//////////////////////////////////////////////////////////////////////
// End conditional_int
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Begin compile time integer comparisons
//////////////////////////////////////////////////////////////////////

template <int A, int B>
using greater_int = conditional_int<(A > B), A, B>;

template <int A, int B>
using lesser_int = conditional_int<(A < B), A, B>;
//////////////////////////////////////////////////////////////////////
// End compile time integer comparisons
//////////////////////////////////////////////////////////////////////

}  // namespace frustum

#endif
