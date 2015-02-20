#ifndef LIBBALLISTAE_SPECTRUM_HH
#define LIBBALLISTAE_SPECTRUM_HH

#include <array>

namespace ballistae
{

struct rgb_tag {};

template<class Elem, class CSTag>
struct color3
{
    std::array<float, 3> channels;
};

template<class Elem, class CSTag>
color3<Elem, CSTag> operator*(const Elem &s, const color3<Elem, CSTag> &c)
{
    auto r = c;
    for(std::size_t i = 0; i < c.channels.size(); ++i)
    {
        r.channels[i] = s * c.channels[i];
    }
    return r;
}

template<class Elem, class CSTag>
color3<Elem, CSTag> operator+(
    const color3<Elem, CSTag> &a,
    const color3<Elem, CSTag> &b
)
{
    auto r = a;
    for(std::size_t i = 0; i < a.channels.size(); ++i)
    {
        r.channels[i] += b.channels[i];
    }
    return r;
}

using color_d_rgb = color3<double, rgb_tag>;

}

#endif
