#ifndef LIBBALLISTAE_SPECTRUM_HH
#define LIBBALLISTAE_SPECTRUM_HH

#include <array>

namespace ballistae
{

struct rgb_tag {};

template<class Elem, class CSTag>
struct color3
{
    std::array<Elem, 3> channels;

    color3<Elem, CSTag>& operator+=(const color3<Elem, CSTag> &other)
    {
        for(std::size_t i = 0; i < this->channels.size(); ++i)
        {
            this->channels[i] += other.channels[i];
        }
        return *this;
    }
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
    r += b;
    return r;
}

using color_d_rgb = color3<double, rgb_tag>;

}

#endif
