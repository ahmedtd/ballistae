
#ifndef FRUSTUM_0_DEFINED_SHIFTS_HH
#define FRUSTUM_0_DEFINED_SHIFTS_HH

// TODO: This is quickly becoming an integer toolbox.  Rename to that effect.

#include <climits>
#include <type_traits>

namespace frustum
{

// Select the wider of two types (not necessarily integral types).
//
// Template parameters:
//
//   * A, B: Types to compare.
//
// Result:
//
//   ::type contains the wider of A and B.
template<class A, class B>
using wider_of = std::conditional<(sizeof(A) > sizeof(B)), A, B>;

// Well-defined shift: In C++, left- and right-shifts of signed negative values
// cause problems.  In particular, left-shifting a signed negative value is
// undefined behavior, and right-shifting a signed negative value is
// implementation-defined behavior.
//
// defined_shift sidesteps these problems using the arithmetic equivalents of
// the shifts for signed operands less than 0.
//
// Caveats:
// 
//  * Undefined Behavior if abs(amount) is greater than or equal to the number
//  * of bits in Type.
//
//  * Undefined behavior if TYPE is a signed type, and the result of the shift
//  * will not fit in TYPE.
// 
// Template parameters:
//
//  * Type: The type of the shiftee.
//
// Runtime parameters:
//
//  * a: The shiftee.
//
//  * amount: Amount to shift.  Positive values imply a left shift, negative
//  * values imply a right shift.  Zero implies no shift.
//
// Return value:
//
//  * a with the appropriate shift applied, preserving sign.
//
// Relevant standard quotes (from the C++11 draft standard):
//
//  * 5.8.2 The value of E1 << E2 is E1 left-shifted E2 bit positions; vacated
//  * bits are zero-filled. If E1 has an unsigned type, the value of the result
//  * is E1 * 2^E2, reduced modulo one more than the maximum value representable
//  * in the result type. Otherwise, if E1 has a signed type and non-negative
//  * value, and E1 * 2^E2 is representable in the corresponding unsigned type
//  * of the result type, then that value, converted to the result type, is the
//  * resulting value; otherwise, the behavior is undefined.
//
//  * 5.8.3 The value of E1 >> E2 is E1 right-shifted E2 bit positions. If E1
//  * has an unsigned type or if E1 has a signed type and a non-negative value,
//  * the value of the result is the integral part of the quotient of
//  * E1/2^E2. If E1 has a signed type and a negative value, the resulting value
//  * is implementation-defined.
//
//  * 4.7.3 If the destination type is signed, the value is unchanged if it can
//  * be represented in the destination type (and bit-field width); otherwise,
//  * the value is implementation-defined.
template<class Type>
constexpr Type defined_shift(Type a, int amount)
{
    return
        (std::is_signed<Type>::value)
        ? ( (amount < 0) ? (a / (1 << (-amount))) : (a * (1 << amount)) )
        : ( (amount < 0) ? (a >> (-amount))       : (a  << amount) );
}

// Less-significant bitmask: Construct a bitmask with ones in all positions [0,
// bit_end).  (Note that the bit at bit_end is not set).  This function is
// constexpr (usable at both compile-time and run-time).
//
// Preconditions:
//
//  * bit_end >= 0: It doesn't mean anything for the mask to start below the
//  * available field.
//
//  * bit_end <= sizeof(Type) * CHAR_BIT : The bitmask must be able to fit in
//  * Type.
//
// Template parameters:
//
//  * Type: An integral type into which the bitmask is constructed.  Must be
//  * large enough to contain the resulting bitmask.
//
// Runtime parameters:
//
//  * bit_end: One-past-the-end index for the bitmask.
//
// Return value:
//
//  * A bitmask, contained within an instance of Type, with all non-padding bits
//  * less-significant than "bit_end" set.
//
// Notes:
// 
//  * The full mask is handled separately, since the general strategy will
//  * produce an intermediary value that is not representible in type.
template<class Type>
constexpr Type make_ls_bitmask(int bit_end)
{
    // TODO (Resolved, but left for illumination): Signed types demand special
    // handling.  For example, a 32-bit unsigned integer cannot accept bit_end
    // == 32 for the normal case, since the value 2^32 will be produced
    // temporarily, but can accept bit_end == 31 (meaning a mask with the
    // highest bit unset).  However, a 32-bit signed integer can handle neither
    // bit_end == 32 nor bit_end == 31, since the intermediate value 2^31 is
    // produced, which is larger than 2^31-1

    return (bit_end == sizeof(Type) * CHAR_BIT)
        
        // Caller wanted full mask
        ? (1 | ~1)
        
        : (bit_end == sizeof(Type) * CHAR_BIT - 1)
        
        // This case is also special, if Type is signed.  In general 2^(b-1)-1
        // is the maximum representible integer for a signed type.  If Type is
        // unsigned, it is also properly handled.
        ? ((1 << (bit_end - 1)) | ((1 << (bit_end - 1)) - 1)) 
  
        // Caller wanted a general mask (also handles bit_end == 0).
        : ((1 << bit_end) - 1);
}

// Bitmask: Construct a bitmask with ones in the range [bit_start, bit_end),
// with the least-significant bit having index 0.
//
// Preconditions:
//
//  * bit_start <= bit_end: 
//
//  * bit_start < sizeof(Type) * CHAR_BIT : It is not legal for the bitmask to
//  * start beyond the end of the bitfield corresponding to "Type".
//
//  * bit_end <= sizeof(Type) * CHAR_BIT : The bitmask must be able to fit in
//  * "Type".
//
//  * bit_end,bit_start >= 0 : 
template<class Type>
constexpr Type make_bitmask(int bit_start, int bit_end)
{
    return make_ls_bitmask<Type>(bit_end) & ~make_ls_bitmask<Type>(bit_start);
}

// Well-defined bitwise rotation: Rotates word by abs(amount) bits (left if
// amount is positive, or right if amount is negative).  The sign bit is not
// modified.
//
// Template parameters:
//
//  * Type: The type to be rotated.  Should be a basic integral type, with
//  * std::is_integral and std::is_signed defined.
//
// Runtime parameters:
//
//  * word: The value to be rotated.
//
//  * amount: The number of bits by which to rotate word.
//
// Return value:
//
//  * "word", rotated by ("amount" % "number of valid rotation bits") bits.  For
//  * a signed type, the number of valid rotation bits is one less than the
//  * number of bits in the type.  For an unsigned type, the number of valid
//  * rotation bits is the number of bits in the type.
template<class Type>
constexpr Type defined_rotate(Type word, int amount)
{
    static_assert(
        std::is_integral<Type>::value,
        "frustum::defined_rotate is only valid for types modeling binary"
        " integer types."
    );

#   define TYPE_BIT (sizeof(Type) * CHAR_BIT)
#   define S_AMOUNT (amount % (TYPE_BIT - 1))
#   define S_OPP_AMOUNT (-((TYPE_BIT - 1 - amount) % (TYPE_BIT - 1)))
#   define U_AMOUNT (amount % TYPE_BIT)
#   define U_OPP_AMOUNT (-((TYPE_BIT - amount) % TYPE_BIT))

    return
        (std::is_signed<Type>::value)
        ? (defined_shift(word, S_AMOUNT) | defined_shift(word, S_OPP_AMOUNT))
        : (defined_shift(word, U_AMOUNT) | defined_shift(word, U_OPP_AMOUNT));

#   undef TYPE_BIT
#   undef S_AMOUNT
#   undef S_OPP_AMOUNT
#   undef U_AMOUNT
#   undef U_OPP_AMOUNT
}

// Split an integral quantity into low and high halves.
//
// Any signed or unsigned integer can be represented in the form
//
//   hi * 2^(b/2) + lo ,
//
// where b is the number of bits in the integer.  defined_split extracts the hi
// and lo components of a given word.  This is particularly useful for extended
// multiplication application, such as frustum's *_multiprec family of types.
//
// Both signed and unsigned integers are handled correctly.  If a negative
// signed integer is split, both the lo and hi components are also negative (so
// that the formula given above holds).
//
// Template parameters:
//
//   * TYPE: The type of the word to split.  Can be inferred from the value
//   * provided for WORD.
//
// Runtime parameters:
//
//   * WORD: The word to split.
//
// Return value:
//
//   A std::array of size 2, where element 0 is lo, and element 1 is hi.
//
// Notes:
//
//   * std::array constexpr behavior is C++14.
template<class Type>
constexpr std::array<Type, 2> defined_split(const Type &word)
{
    const std::array<Type, 2> splitword;
    
    const int whole_bits = (CHAR_BIT * sizeof(Type));
    const int half_bits = whole_bits / 2;

    const Type min = std::numeric_limits<Type>::min();

    if(word < 0)
    {
        splitword[0] = (min & make_bitmask(half_bits, whole_bits))
            | (word & make_bitmask<Type>(0, half_bits));
        splitword[1] = defined_shift(word, -half_bits);
    }
    else
    {
        splitword[0] = word & make_ls_bitmask<Type>(half_bits);
        splitword[1] = defined_shift(word, -half_bits);
    }

    return splitword; 
}

// Wide (full-result) addition.
//
// Adds two numbers (possibly of different widths), and returns the sum (modulo
// 2^b) and carry.
//
// Template parameters:
//
//   * TYPEA, TYPEB, TYPEC (integral type): The types of A, B, and
//   * CARRY, respectively.
//
// Runtime parameters:
//
//   * A (TYPEA), B (TYPEB): The numbers to add.
//
//   * CARRY (TYPEC): The carry input.
//
// Return value:
//
//   A std::array of the wider of TYPEA and TYPEB.  The array has two elements.
//   Element 0 contains the sum of A and B module 2^b.  Element 1 contains the
//   single carry bit.
//
// Caveats:
//
//   * Don't mix signed and unsigned types in the arguments.
template<class TypeA, class TypeB>
constexpr std::array<wider_of<TypeA, TypeB>::type, 2> wide_add(
    const TypeA &a,
    const TypeB &b,
    const TypeC &carry = 0
) {
    static_assert(
        std::is_integral<TypeA>::value && std::is_integral<TypeB>::value,
        "frustum::wide_add: Both arguments must be integral."
    );

    static_assert(
        std::is_signed<TypeA>::value == std::is_signed<TypeB>::value,
        "frustum::wide_add: Both arguments must have the same signedness."
    );

    typedef wider_of<TypeA, TypeB>::type WorkType;
    
    const int half_bits = (CHAR_BIT * sizeof(WorkType)) / 2;

    auto a_split = defined_split<WorkType>(a);
    auto b_split = defined_split<WorkType>(b);
    
    auto lo_result = defined_split(a_split[0] + b_split[0] + carry);
    auto hi_result = defined_split(
        a_split[1] + b_split[1] + lo_result[1]
    );
    
    std::array<WorkType, 2> result;
    result[0] = lo_result[0] + defined_shift(hi_result[0], half_bits);
    result[1] = hi_result[1];
}

// Wide (full-result) multiplication.
//
// Caveats:
//
//   * Don't try to mix signed and unsigned types here.
template<class TypeA, class TypeB>
constexpr std::array<wider_of<TypeA, TypeB>::type, 2> wide_mult(
    const TypeA &a,
    const TypeB &b
) {
    static_assert(
        std::is_signed<TypeA>::value == std::is_signed<TypeB>::value,
        "frustum::wide_mult: Both arguments must have the same signedness."
    );

    typedef wider_of<TypeA, TypeB>::type WorkType;

    // We split using the width of the larger type.  This causes an implicit
    // conversion from both types to the larger type.
    auto a_split = defined_split<WorkType>(a);
    auto b_split = defined_split<WorkType>(b);

    
}

}

#endif
