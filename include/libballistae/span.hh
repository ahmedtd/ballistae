#ifndef LIBBALLISTAE_SPAN_HH
#define LIBBALLISTAE_SPAN_HH

#include <limits>

#include <armadillo>

namespace ballistae
{

template<class Field>
struct span final
{
    using vec_type = typename arma::Col<Field>::template fixed<3>;

    Field lo;
    Field hi;

    vec_type lo_normal;
    vec_type hi_normal;

    static span<Field> pos_half()
    {
        constexpr Field inf = std::numeric_limits<Field>::infinity();
        constexpr Field nan = std::numeric_limits<Field>::quiet_NaN();

        return {Field(0), inf, {nan, nan, nan}, {nan, nan, nan}};
    }

    static span<Field> inf()
    {
        constexpr Field inf = std::numeric_limits<Field>::infinity();
        constexpr Field nan = std::numeric_limits<Field>::quiet_NaN();

        return {inf, inf, {nan, nan, nan}, {nan, nan, nan}};
    }

    static span<Field> nan()
    {
        constexpr double nan = std::numeric_limits<Field>::quiet_NaN();
        return {nan, nan, {nan, nan, nan}, {nan, nan, nan}};
    }

    static span<Field> undecorated(const Field &lo, const Field &hi)
    {
        span<Field> result;
        result.lo = lo;
        result.hi = hi;
        return result;
    }
};

template<class Field>
bool overlaps(const span<Field> &a, const span<Field> &b)
{
    return !(a.lo >= b.hi || a.hi <= b.lo);
}

template<class Field>
bool contains(const span<Field> &a, const span<Field> &b)
{
    return a.lo <= b.lo && a.hi > b.hi;
}

template<class Field>
bool operator<(const span<Field> &a, const span<Field> &b)
{
    if(a.lo != b.lo)
        return a.lo < b.lo;
    else
        return a.hi < b.hi;
}

}

#endif
