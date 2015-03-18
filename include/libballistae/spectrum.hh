#ifndef LIBBALLISTAE_SPECTRUM_HH
#define LIBBALLISTAE_SPECTRUM_HH

#include <cmath>

#include <algorithm>
#include <array>
#include <vector>

#include <armadillo>

namespace ballistae
{

template<class Field, size_t N>
struct fixed_dense_function
{
    Field src_val;
    Field lim_val;

    std::array<Field, N> samples;

    Field support() const;
    Field operator()(Field x) const;
};

template<class Field, size_t N>
Field fixed_dense_function<Field, N>::support() const
{
    return (lim_val - src_val) / Field(samples.size());
}

/// Interpolate the sampled function at [x].
template<class Field, size_t N>
Field fixed_dense_function<Field, N>::operator()(Field x) const
{
    using std::floor;

    if(x < src_val || x >= lim_val)
    {
        return 0;
    }
    else
    {
        long index = std::lrint(std::floor((x - src_val) / support()));

        // Result is guaranteed to be a valid index into samples.
        return samples[index];
    }
}

/// Take part of the inner product between [cmf] and a sampled function.
///
/// This lets us only store an XYZ color for each pixel, rather than the full
/// spectrum.  As we perform spectral sampling, each sample is immediately
/// folded into the current XYZ val.
template<class Field, size_t N>
Field partial_inner_product(
    fixed_dense_function<Field, N> cmf,
    Field sample_x,
    Field sample_y
)
{
    return cmf(sample_x) * sample_y * cmf.support();
}

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
    return {
        max(Elem(0), a.channels[0]),
        max(Elem(0), a.channels[1]),
        max(Elem(0), a.channels[2])
    };
}

template<class Field>
color3<Field, rgb_tag> to_rgb(const color3<Field, XYZ_tag> &src)
{
    return {{{
         3.240479 * src[0] + -1.537150 * src[1] + -0.498535 * src[2],
        -0.969265 * src[0] +  1.875992 * src[1] +  0.041556 * src[2],
         0.055648 * src[0] + -0.204043 * src[1] +  1.057311 * src[2]
             }}};
}

using color_d_rgb = color3<double, rgb_tag>;
using color_d_XYZ = color3<double, XYZ_tag>;

}

#include <libballistae/cmf_cie_2006.hh>

namespace ballistae
{

template<class Field>
color3<Field, XYZ_tag> spectral_to_XYZ(const Field &x, const Field &y)
{
    color3<Field, XYZ_tag> result;
    result.channels[0] = partial_inner_product(cie_2006_X<Field>(), x, y);
    result.channels[1] = partial_inner_product(cie_2006_Y<Field>(), x, y);
    result.channels[2] = partial_inner_product(cie_2006_Z<Field>(), x, y);
    return result;
}

}

#endif
