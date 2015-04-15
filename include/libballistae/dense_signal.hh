#ifndef LIBBALLISTAE_DENSE_SIGNAL_HH
#define LIBBALLISTAE_DENSE_SIGNAL_HH

#include <cmath>

#include <algorithm>
#include <vector>

namespace ballistae
{

template<class Field>
struct dense_signal
{
    Field src_val;
    Field lim_val;

    std::vector<Field> samples;

    size_t size() const;

    Field support() const;

    Field& operator()(Field x);
    const Field& operator()(Field x) const;

    Field& operator[](size_t i);
    const Field& operator[](size_t i) const;
};

template<class Field>
size_t dense_signal<Field>::size() const
{
    return samples.size();
}

template<class Field>
Field dense_signal<Field>::support() const
{
    return (lim_val - src_val) / Field(samples.size());
}

template<class Field>
Field& dense_signal<Field>::operator()(Field x)
{
    using std::floor;

    long index = std::lrint(std::floor((x - src_val) / support()));

    // Result is guaranteed to be a valid index into samples.
    return samples[index];
}

template<class Field>
const Field& dense_signal<Field>::operator()(Field x) const
{
    return const_cast<dense_signal<Field>*>(this)->operator()(x);
}

template<class Field>
Field& dense_signal<Field>::operator[](size_t i)
{
    return samples[i];
}

template<class Field>
const Field& dense_signal<Field>::operator[](size_t i) const
{
    return const_cast<dense_signal<Field>*>(this)->operator[](i);
}

////////////////////////////////////////////////////////////////////////////////
/// Arithmetic operators on signals.
////////////////////////////////////////////////////////////////////////////////

template<class FieldA, class FieldB>
bool operator==(const dense_signal<FieldA> &a, const dense_signal<FieldB> &b)
{
    return std::equal(a.samples.cbegin(), a.samples.cend(), b.samples.cbegin());
}

template<class Field>
dense_signal<Field> operator+(
    const dense_signal<Field> &a,
    const dense_signal<Field> &b
)
{
    dense_signal<Field> result = a;
    for(size_t i = 0; i < b.samples.size(); ++i)
        result[i] = result[i] + b[i];
    return result;
}

template<class FieldA, class FieldB>
auto operator*(
    const FieldA &a,
    const dense_signal<FieldB> &b
)
{
    dense_signal<decltype(a * b.samples[0])> result = b;
    for(size_t i = 0; i < b.samples.size(); ++i)
        result[i] = a * b[i];
    return result;
}


template<class Field>
dense_signal<Field> operator*(
    const dense_signal<Field> &a,
    const dense_signal<Field> &b
)
{
    dense_signal<Field> result = a;
    for(size_t i = 0; i < b.samples.size(); ++i)
        result[i] = result[i] * b[i];
    return result;
}

/// Take part of the inner product between [sig] and a sampled function.
///
/// This lets us only store an XYZ color for each pixel, rather than the full
/// spectrum.  As we perform spectral sampling, each sample is immediately
/// folded into the current XYZ val.
template<class Field>
Field partial_inner_product(
    const dense_signal<Field>& sig,
    Field sample_x,
    Field sample_y
)
{
    return sig(sample_x) * sample_y * sig.support();
}

////////////////////////////////////////////////////////////////////////////////
/// Create an empty signal suitable for representing visible spectra.
////////////////////////////////////////////////////////////////////////////////

template<class Field>
dense_signal<Field> vis_spectrum_signal()
{
    std::vector<Field> samples(89, Field(0));
    return {390, 835, std::move(samples)};
}

////////////////////////////////////////////////////////////////////////////////
/// Some useful spectra.
////////////////////////////////////////////////////////////////////////////////

/// Place a pulse into [sig].
template<class Field>
dense_signal<Field> pulse(
    Field sigsrc, Field siglim,
    size_t n,
    Field pulsrc, Field pullim,
    Field val
)
{
    dense_signal<Field> sig = {sigsrc, siglim, std::vector<Field>(n, Field(0))};

    double x = sig.src_val;
    for(Field &y : sig.samples)
    {
        if(pulsrc <= x && x < pullim)
            y += val;

        x += sig.support();
    }

    return sig;
}

/// Add a delta into [sig].
template<class Field>
dense_signal<Field> delta(
    Field sigsrc, Field siglim,
    size_t n,
    Field x, Field val
)
{
    dense_signal<Field> sig = {sigsrc, siglim, std::vector<Field>(n, Field(0))};
    sig(x) += val;
    return sig;
}

////////////////////////////////////////////////////////////////////////////////
/// Visible spectra suitable for representing primary colors.
////////////////////////////////////////////////////////////////////////////////
///
/// Often, input data will be specified in an RGB color space, but there are an
/// infinite number of spectra that correspond to any given RGB triple.
///
/// These functions provide suitable default choices to form a spectrum from an
/// RGB triple:
///
///     R * red() + G * green() + B * blue()

template<class Field>
const dense_signal<Field>& red()
{
    static const dense_signal<Field> sig = pulse(
        Field(390),
        Field(835),
        89,
        Field(620),
        Field(750),
        Field(1)
    );

    return sig;
}

template<class Field>
const dense_signal<Field>& blue()
{
    static const dense_signal<Field> sig = pulse(
        Field(390),
        Field(835),
        89,
        Field(420),
        Field(495),
        Field(1)
    );

    return sig;
}

template<class Field>
const dense_signal<Field>& green()
{
    static const dense_signal<Field> sig = pulse(
        Field(390),
        Field(835),
        89,
        Field(495),
        Field(570),
        Field(1)
    );

    return sig;
}

////////////////////////////////////////////////////////////////////////////////
/// CIE 2006 XYZ color matching functions.
////////////////////////////////////////////////////////////////////////////////
///
/// Use with [inner_product] or [partial_inner_product] to convert a measured
/// power spectrum to the XYZ color space.

template<class Field>
const dense_signal<Field>& cie_2006_X()
{
    static const std::vector<Field> vals = {
        0.003769647,   0.009382967,   0.02214302,    0.04742986,    0.08953803,
        0.1446214,     0.2035729,     0.2488523,     0.2918246,     0.3227087,
        0.3482554,     0.3418483,     0.3224637,     0.2826646,     0.2485254,
        0.2219781,     0.1806905,     0.129192,      0.08182895,    0.04600865,
        0.02083981,    0.007097731,   0.002461588,   0.003649178,   0.01556989,
        0.04315171,    0.07962917,    0.1268468,     0.1818026,     0.2405015,
        0.3098117,     0.3804244,     0.4494206,     0.5280233,     0.6133784,
        0.7016774,     0.796775,      0.8853376,     0.9638388,     1.051011,
        1.109767,      1.14362,       1.151033,      1.134757,      1.083928,
        1.007344,      0.9142877,     0.8135565,     0.6924717,     0.575541,
        0.4731224,     0.3844986,     0.2997374,     0.2277792,     0.1707914,
        0.1263808,     0.09224597,    0.0663996,     0.04710606,    0.03292138,
        0.02262306,    0.01575417,    0.01096778,    0.00760875,    0.005214608,
        0.003569452,   0.002464821,   0.001703876,   0.001186238,   0.0008269535,
        0.0005758303,  0.0004058303,  0.0002856577,  0.0002021853,  0.000143827,
        0.0001024685,  7.347551E-005, 5.25987E-005,  3.806114E-005, 2.758222E-005,
        2.004122E-005, 1.458792E-005, 1.068141E-005, 7.857521E-006, 5.768284E-006,
        4.259166E-006, 3.167765E-006, 2.358723E-006, 1.762465E-006
    };

    static const dense_signal<Field> sig = {390, 835, vals};
    return sig;
}

template<class Field>
const dense_signal<Field>& cie_2006_Y()
{
    static const std::vector<Field> vals = {
        0.0004146161,  0.001059646,   0.002452194,   0.004971717,   0.00907986,
        0.01429377,    0.02027369,    0.02612106,    0.03319038,    0.0415794,
        0.05033657,    0.05743393,    0.06472352,    0.07238339,    0.08514816,
        0.1060145,     0.1298957,     0.1535066,     0.1788048,     0.2064828,
        0.237916,      0.285068,      0.3483536,     0.4277595,     0.5204972,
        0.6206256,     0.718089,      0.7946448,     0.8575799,     0.9071347,
        0.9544675,     0.9814106,     0.9890228,     0.9994608,     0.9967737,
        0.9902549,     0.9732611,     0.9424569,     0.8963613,     0.8587203,
        0.8115868,     0.7544785,     0.6918553,     0.6270066,     0.5583746,
        0.489595,      0.4229897,     0.3609245,     0.2980865,     0.2416902,
        0.1943124,     0.1547397,     0.119312,      0.08979594,    0.06671045,
        0.04899699,    0.03559982,    0.02554223,    0.01807939,    0.01261573,
        0.008661284,   0.006027677,   0.004195941,   0.002910864,   0.001995557,
        0.001367022,   0.0009447269,  0.000653705,   0.000455597,   0.0003179738,
        0.0002217445,  0.0001565566,  0.0001103928,  7.827442E-005, 5.578862E-005,
        3.981884E-005, 2.860175E-005, 2.051259E-005, 1.487243E-005, 0.0000108,
        7.86392E-006,  5.736935E-006, 4.211597E-006, 3.106561E-006, 2.286786E-006,
        1.693147E-006, 1.262556E-006, 9.422514E-007, 7.05386E-007
    };

    static const dense_signal<Field> sig = {390, 835, vals};
    return sig;
}

template<class Field>
const dense_signal<Field>& cie_2006_Z()
{
    static const std::vector<double> vals = {
        0.0184726,     0.04609784,    0.109609,      0.2369246,     0.4508369,
        0.7378822,     1.051821,      1.305008,      1.552826,      1.74828,
        1.917479,      1.918437,      1.848545,      1.664439,      1.522157,
        1.42844,       1.25061,       0.9991789,     0.7552379,     0.5617313,
        0.4099313,     0.3105939,     0.2376753,     0.1720018,     0.1176796,
        0.08283548,    0.05650407,    0.03751912,    0.02438164,    0.01566174,
        0.00984647,    0.006131421,   0.003790291,   0.002327186,   0.001432128,
        0.0008822531,  0.0005452416,  0.0003386739,  0.0002117772,  0.0001335031,
        8.494468E-005, 5.460706E-005, 3.549661E-005, 2.334738E-005, 1.554631E-005,
        1.048387E-005, 0,             0,             0,             0,
        0,             0,             0,             0,             0,
        0,             0,             0,             0,             0,
        0,             0,             0,             0,             0,
        0,             0,             0,             0,             0,
        0,             0,             0,             0,             0,
        0,             0,             0,             0,             0,
        0,             0,             0,             0,             0,
        0,             0,             0,             0
    };

    static const dense_signal<Field> sig = {390, 835, vals};
    return sig;
}

}

#endif
