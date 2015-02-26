#ifndef LIBBALLISTAE_SPAN_HH
#define LIBBALLISTAE_SPAN_HH

#include <limits>

namespace ballistae
{

template<class Field>
struct span final
{
    Field lo;
    Field hi;

    static Field inf()
    {
        return std::numeric_limits<Field>::infinity();
    }

    static span<Field> nan()
    {
        return {
            std::numeric_limits<Field>::quiet_NaN(),
            std::numeric_limits<Field>::quiet_NaN()
        };
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
