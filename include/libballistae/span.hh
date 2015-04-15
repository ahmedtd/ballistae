#ifndef LIBBALLISTAE_SPAN_HH
#define LIBBALLISTAE_SPAN_HH

#include <array>
#include <limits>

namespace ballistae
{

template<class Field>
struct span final
{
    Field lo;
    Field hi;

    static span<Field> pos_half()
    {
        constexpr Field inf = std::numeric_limits<Field>::infinity();
        return {Field(0), inf};
    }

    static span<Field> nan()
    {
        return {
            std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::quiet_NaN()
        };
    }
};

template<class Field>
Field measure(const span<Field> &a)
{
    return a.hi - a.lo;
}

template<class Field>
bool overlaps(const span<Field> &a, const span<Field> &b)
{
    return !(a.lo > b.hi || a.hi < b.lo);
}


template<class Field>
bool contains(const span<Field> &a, const span<Field> &b)
{
    return a.lo <= b.lo && a.hi >= b.hi;
}

template<class Field>
bool contains(const span<Field> &a, const Field &b)
{
    return a.lo <= b && b <= a.hi;
}

template<class Field>
bool operator<(const span<Field> &a, const span<Field> &b)
{
    if(a.lo != b.lo)
        return a.lo < b.lo;
    else
        return a.hi < b.hi;
}

template<class Field>
bool strictly_precedes(const span<Field> &a, const Field &b)
{
    return a.hi < b;
}

template<class Field>
bool strictly_succeeds(const span<Field> &a, const Field &b)
{
    return a.lo > b;
}

/// Smallest span that contains A and B.
template<class Field>
span<Field> min_containing(const span<Field> &a, const span<Field> &b)
{
    using std::max;
    using std::min;
    return {min(a.lo, b.lo), max(a.hi, b.hi)};
}

/// Largest span covered by both A and B.
///
/// Preconditions:
///
///   overlaps(a, b)
template<class Field>
span<Field> max_intersecting(const span<Field> &a, const span<Field> &b)
{
    using std::max;
    using std::min;
    return {max(a.lo, b.lo), min(a.hi, b.hi)};
}

/// Split A into two spans at CUT.
///
/// Preconditions:
///
///   * contains(a, cut)
template<class Field>
std::array<span<Field>, 2> cut(const span<Field> &a, const Field &cut)
{
    span<Field> lo = a;
    span<Field> hi = a;

    lo.hi = cut;
    hi.lo = cut;

    return {lo, hi};
}

template<class FieldA, class FieldB>
auto operator*(const FieldA &a, const span<FieldB> b)
{
    // TODO: think carefully about multiplication by a negative number.
    span<decltype(a * b.lo)> result = {a * b.lo, a * b.hi};
    return result;
}

}

#endif
