#ifndef FRUSTUM_0_COMPILE_TIME_OPERATIONS_HH
#define FRUSTUM_0_COMPILE_TIME_OPERATIONS_HH

#include <cassert>

namespace frustum
{

namespace ct_ops
{

template<class ArgsHead>
constexpr auto product(ArgsHead head)
{
    return head;
}

template<class ArgsHead, class ArgsSecond, class... ArgsTail>
constexpr auto product(
    ArgsHead head,
    ArgsSecond second,
    ArgsTail... tail
) {
    return head * product(second, tail...);
}

template<class Arg>
constexpr auto select_ith(
    size_t i,
    Arg head
)
{
    return head;
}

template<class ArgsHead, class ArgsSecond, class... ArgsTail>
constexpr auto select_ith(
    size_t i,
    ArgsHead head,
    ArgsSecond second,
    ArgsTail... tail
)
{
    return (i == 0)
        ? head
        : select_ith(i-1, second, tail...);
}

// A small helper class that produces a false value that is dependent on the the
// template arguments.  This is needed because a static_assert with a constant
// false argument will fail as soon as the compiler sees it.  By making it
// dependent on the template arguments, we can create a static_assert that will
// be triggered only when the template is instantiated.
template<class... Dummies>
struct false_helper
{
    static const bool value = false;
};

template<class... Dummies>
struct true_helper
{
    static const bool value = true;
};

// Copy an argpack into an appropriately-sized range.
template<class ArgHeadType, class... ArgTailTypes>
struct argpack_to_range
{
    template<class RangeIt>
    static RangeIt work(
        RangeIt output_begin,
        ArgHeadType arg_head,
        ArgTailTypes... arg_tail
    )
    {
        *output_begin = arg_head;
        return argpack_to_range<ArgTailTypes...>::work(
            output_begin + 1,
            arg_tail...
        );
    }
};

template<class ArgFinalType>
struct argpack_to_range<ArgFinalType>
{
    template<class RangeIt>
    static RangeIt work(
        RangeIt output_begin,
        ArgFinalType arg_final
    )
    {
        *output_begin = arg_final;
        return output_begin + 1;
    }
};

template<class T, T... Ints>
struct integer_sequence
{
    typedef T value_type;

    using type = integer_sequence<T, Ints...>;

    static constexpr std::size_t size()
    {
        return sizeof...(Ints);
    }
};

template<class T1, class T2>
struct integer_sequence_construct
{
};

template<class Int, Int... Seq1, Int... Seq2>
struct integer_sequence_construct<
    integer_sequence<Int, Seq1...>,
    integer_sequence<Int, Seq2...>
    > : integer_sequence<Int, Seq1..., (sizeof...(Seq1) + Seq2)...>
{
};

template<class Int, Int N>
struct make_integer_sequence
    : integer_sequence_construct<
    typename make_integer_sequence<Int, N/2>::type,
    typename make_integer_sequence<Int, N - N/2>::type
    >
{
};

// Base case specializations.  I have to have one for each type I want
// to support, since the obvious strategy fails due to the literal
// arguments being of templated type.

#define MAKE_SPECIALIZATIONS(ARG)                                       \
    template<>                                                          \
    struct make_integer_sequence<ARG, 0> : integer_sequence<ARG> {};    \
    template<>                                                          \
    struct make_integer_sequence<ARG, 1> : integer_sequence<ARG, 0> {};

MAKE_SPECIALIZATIONS(int)
MAKE_SPECIALIZATIONS(std::size_t)

#undef MAKE_SPECIALIZATIONS

template<std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

template<std::size_t... Ints>
using make_index_sequence = make_integer_sequence<std::size_t, Ints...>;

// Same as an index sequence, but using "index" where we mean "dimension" could
// be confusing when reading the code.
template<std::size_t... Ints>
using dim_sequence = integer_sequence<std::size_t, Ints...>;

template<bool Arg>
struct bool_to_type
{
    static constexpr bool value = Arg;
};

template<int Arg>
struct int_to_type
{
    static constexpr int value = Arg;
};

// Variadic template max function (base case).
template<class TailType>
constexpr auto max(const TailType &tail)
{
    return tail;
}

// Variadic template max function.
//
// Don't invoke it on a nonuniform list of types.
template<class HeadType, class... TailTypes>
constexpr HeadType max(const HeadType &head, const TailTypes&... tail)
{
    return (head > max(tail...)) ? head : max(tail...);
}

template<class ArgsHead, class... ArgsTail>
constexpr bool all_true(ArgsHead h, ArgsTail... t)
{
    return h && all_true(t...);
}

// Close namespace frustum::ct_ops
}

}

#endif
