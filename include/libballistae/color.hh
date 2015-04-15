#ifndef LIBBALLISTAE_SPECTRUM_HH
#define LIBBALLISTAE_SPECTRUM_HH

#include <cmath>

#include <algorithm>
#include <array>
#include <vector>

#include <armadillo>

namespace ballistae
{

struct rgb_tag {};
struct XYZ_tag {};

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

    Elem& operator[](std::size_t i)
    {
        return channels[i];
    }

    const Elem& operator[](std::size_t i) const
    {
        return channels[i];
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

template<class Elem, class CSTag>
color3<Elem, CSTag> clamp_0(const color3<Elem, CSTag> &a)
{
    using std::max;

    color3<Elem, CSTag> result = {
        max(Elem(0), a.channels[0]),
        max(Elem(0), a.channels[1]),
        max(Elem(0), a.channels[2])
    };
    return result;
}

template<class Field>
color3<Field, rgb_tag> to_rgb(const color3<Field, XYZ_tag> &src)
{
    color3<Field, rgb_tag> result = {
        3.240479 * src[0] + -1.537150 * src[1] + -0.498535 * src[2],
        -0.969265 * src[0] +  1.875992 * src[1] +  0.041556 * src[2],
        0.055648 * src[0] + -0.204043 * src[1] +  1.057311 * src[2]
    };
    return result;
}

using color_d_rgb = color3<double, rgb_tag>;
using color_d_XYZ = color3<double, XYZ_tag>;

template<class Field>
color3<Field, XYZ_tag> spectral_to_XYZ(const Field &x, const Field &y)
{
    color3<Field, XYZ_tag> result;
    result.channels[0] = partial_inner_product(cie_2006_X<Field>(), x, y);
    result.channels[1] = partial_inner_product(cie_2006_Y<Field>(), x, y);
    result.channels[2] = partial_inner_product(cie_2006_Z<Field>(), x, y);
    return result;
}

template<class Field>
dense_signal<Field> rgb_to_spectral(const color3<Field, rgb_tag> &src)
{
    return src[0] * red<Field>()
        + src[1] * green<Field>()
        + src[2] * blue<Field>();
}

}

#endif
