#include <libballistae/material_map.hh>

#include <cmath>

#include <utility>

#include <libballistae/vector_distributions.hh>

namespace ballistae
{

constant_mtlmap1::constant_mtlmap1(dense_signal<double> spectrum_in)
    : spectrum(std::move(spectrum_in))
{
}

constant_mtlmap1::~constant_mtlmap1()
{
}

fixvec<double, 1> constant_mtlmap1::value(
    const fixvec<double, 2> &mtl2,
    const fixvec<double, 3> &mtl3,
    double lambda
) const
{
    return {interpolate(spectrum, lambda)};
}

template<class Scalar, class QA, class QB>
auto lerp(Scalar t, QA a, QB b)
{
    return (1 - t) * a + t * b;
}

lerp_mtlmap1::lerp_mtlmap1(
    double t_lo_in,
    double t_hi_in,
    const mtlmap<1> *t_in,
    const mtlmap<1> *a_in,
    const mtlmap<1> *b_in
)
    : t_lo(t_lo_in),
      t_hi(t_hi_in),
      t(t_in),
      a(a_in),
      b(b_in)
{
}

fixvec<double, 1> lerp_mtlmap1::value(
    const fixvec<double, 2> &mtl2,
    const fixvec<double, 3> &mtl3,
    double lambda
) const
{
    double tval = t->value(mtl2, mtl3, lambda)(0);

    tval -= t_lo;
    tval /= t_hi - t_lo;

    if(tval < t_lo)
        tval = t_lo;
    if(tval > t_hi)
        tval = t_hi;

    return lerp(
        tval,
        a->value(mtl2, mtl3, lambda),
        b->value(mtl2, mtl3, lambda)
    );
}

level_mtlmap1::level_mtlmap1(
    double t_lo_in,
    double t_hi_in,
    const mtlmap<1> *t_in,
    const mtlmap<1> *a_in,
    const mtlmap<1> *b_in
)
    : t_lo(t_lo_in),
      t_hi(t_hi_in),
      t(t_in),
      a(a_in),
      b(b_in)
{
}

fixvec<double, 1> level_mtlmap1::value(
    const fixvec<double, 2> &mtl2,
    const fixvec<double, 3> &mtl3,
    double lambda
) const
{
    double tval = t->value(mtl2, mtl3, lambda)(0);

    tval -= t_lo;
    tval /= t_hi - t_lo;

    if(tval < 0.5)
        return a->value(mtl2, mtl3, lambda);
    else
        return b->value(mtl2, mtl3, lambda);
}

checkerboard_mtlmap1::checkerboard_mtlmap1(
    double period_in,
    bool volumetric_in
)
    : period(period_in),
      volumetric(volumetric_in)
{
}

checkerboard_mtlmap1::~checkerboard_mtlmap1()
{
}

template<size_t D>
bool parity(const fixvec<double, D> &mtl, double period)
{
    using std::floor;

    unsigned char parity = 0;
    for(size_t i = 0; i < D; ++i)
    {
        double q = mtl(i) / period;
        double f = floor(q);

        double rem = q - f;

        unsigned char cur = (rem > 0.5) ? 1 : 0;
        parity ^= cur;
    }

    return parity & 0x1u;
}

fixvec<double, 1> checkerboard_mtlmap1::value(
    const fixvec<double, 2> &mtl2,
    const fixvec<double, 3> &mtl3,
    double lambda
) const
{
    fixvec<double, 1> v0 = {0};
    fixvec<double, 1> v1 = {1};

    return
        (volumetric ? parity(mtl3, period) : parity(mtl2, period))
        ? v1
        : v0;
}

// A multiplicative hash (in Knuth's style), that makes use of the fact that we
// only use 24 input bits.
//
// The multiplicative constant is floor(2^24 / (golden ratio)), tweaked a bit to
// avoid attractors above 0xc in the last digit.
uint32_t hashmul(uint32_t x) __attribute__((visibility("hidden")));
uint32_t hashmul(uint32_t x)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x);
    return x;
}

// static const uint32_t p [] = {
//     151,160,137,91,90,15,
//     131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
//     190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
//     88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
//     77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
//     102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
//     135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
//     5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
//     223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
//     129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
//     251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
//     49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
//     138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
//     151,160,137,91,90,15,
//     131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
//     190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
//     88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
//     77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
//     102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
//     135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
//     5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
//     223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
//     129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
//     251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
//     49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
//     138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
// };

template<class Field>
Field perlin_dotgrad(fixvec<size_t, 3> c, fixvec<Field, 3> d)
{
    // Map the low four bits hash, and map them to one of the 12 unit integer
    // gradients.  Then, take the dot product of d with the selected gradient.
    // This is an improvement from Perlin's 2002 paper.

    uint32_t hash
        = ((c(0) & 0xffu) << 16)
        | ((c(1) & 0xffu) <<  8)
        | ((c(2) & 0xffu) <<  0);

    hash = hashmul(hash);

    switch(hash & 0xfu)
    {
    case 0x0u: return  d(0) + d(1);
    case 0x1u: return  d(0) - d(1);
    case 0x2u: return -d(0) + d(1);
    case 0x3u: return -d(0) - d(1);

    case 0x4u: return  d(1) + d(2);
    case 0x5u: return  d(1) - d(2);
    case 0x6u: return -d(1) + d(2);
    case 0x7u: return -d(1) - d(2);

    case 0x8u: return  d(2) + d(0);
    case 0x9u: return  d(2) - d(0);
    case 0xau: return -d(2) + d(0);
    case 0xbu: return -d(2) - d(0);

    case 0xcu: return  d(0) + d(1);
    case 0xdu: return -d(0) + d(1);
    case 0xeu: return -d(1) + d(2);
    case 0xfu: return -d(1) - d(2);
    };

    return 0; // Dead code.
}

template<class Field>
Field fade(Field x)
{
    return x * x * x * (x * (x * 6 - 15) + 10);
}

template<class Field>
Field perlin_val(const fixvec<Field, 3> &x_abs)
{
    using std::floor;
    using std::lrint;

    using iv = fixvec<size_t, 3>;
    using fv = fixvec<Field, 3>;

    fixvec<long, 3> cell_long = lrint(floor(x_abs));
    fixvec<size_t, 3> cell = {
        static_cast<size_t>(cell_long(0) & 0xff),
        static_cast<size_t>(cell_long(1) & 0xff),
        static_cast<size_t>(cell_long(2) & 0xff)
    };
    fixvec<Field, 3> x_rel = x_abs - floor(x_abs);

    Field val =
        lerp(fade(x_rel(2)),
             lerp(fade(x_rel(1)),
                  lerp(fade(x_rel(0)),
                       perlin_dotgrad(cell + iv{0,0,0}, x_rel - fv{0, 0, 0}),
                       perlin_dotgrad(cell + iv{1,0,0}, x_rel - fv{1, 0, 0})),
                  lerp(fade(x_rel(0)),
                       perlin_dotgrad(cell + iv{0,1,0}, x_rel - fv{0, 1, 0}),
                       perlin_dotgrad(cell + iv{1,1,0}, x_rel - fv{1, 1, 0}))),
             lerp(fade(x_rel(1)),
                  lerp(fade(x_rel(0)),
                       perlin_dotgrad(cell + iv{0,0,1}, x_rel - fv{0, 0, 1}),
                       perlin_dotgrad(cell + iv{1,0,1}, x_rel - fv{1, 0, 1})),
                  lerp(fade(x_rel(0)),
                       perlin_dotgrad(cell + iv{0,1,1}, x_rel - fv{0, 1, 1}),
                       perlin_dotgrad(cell + iv{1,1,1}, x_rel - fv{1, 1, 1}))));

    return val;
}

perlinval_mtlmap1::perlinval_mtlmap1(
    bool volumetric_in,
    double period_in
)
    : volumetric(volumetric_in),
      period(period_in)
{
}

perlinval_mtlmap1::~perlinval_mtlmap1()
{
}

fixvec<double, 1> perlinval_mtlmap1::value(
    const fixvec<double, 2> &mtl2,
    const fixvec<double, 3> &mtl3,
    double lambda
) const
{
    fixvec<double, 3> mtl = volumetric
        ? mtl3 * 256.0 / period
        : fixvec<double, 3>{mtl2(0) * 256.0 / period, mtl2(1) * 256.0 / period, 1.0};

    auto val = perlin_val(mtl);
    return {val};
}

}
