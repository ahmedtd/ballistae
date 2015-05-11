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

checkerboard_mtlmap1::checkerboard_mtlmap1(
    const mtlmap<1> *even_mtlmap_in,
    const mtlmap<1> *odd_mtlmap_in,
    double period_in,
    bool volumetric_in
)
    : even_mtlmap(even_mtlmap_in),
      odd_mtlmap(odd_mtlmap_in),
      period(period_in),
      volumetric(volumetric_in)
{
}

checkerboard_mtlmap1::~checkerboard_mtlmap1()
{
}

template<size_t D>
bool parity(const fixvec<double, D> &mtl, double period)
{
    using std::fmod;

    unsigned char parity = 0;
    for(size_t i = 0; i < D; ++i)
    {
        double rem = fmod(mtl(i), period) / period;

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
    return
        (volumetric ? parity(mtl3, period) : parity(mtl2, period))
        ? even_mtlmap->value(mtl2, mtl3, lambda)
        : odd_mtlmap->value(mtl2, mtl3, lambda);
}

perlinval2_mtlmap1::perlinval2_mtlmap1(double scale_in, size_t seed)
    : scale(scale_in)
{
    using std::begin;
    using std::end;
    using std::generate;

    std::mt19937 g(seed);
    uniform_unitv_distribution<double, 2> dist;
    generate(begin(kernel), end(kernel), [&](){return dist(g);});
}

perlinval2_mtlmap1::~perlinval2_mtlmap1()
{
}

size_t kernel_idx(size_t i, size_t j)
{
    return (i & 0xffu) << 8 | (j & 0xffu);
}

fixvec<double, 1> perlinval2_mtlmap1::value(
    const fixvec<double, 2> &mtl2,
    const fixvec<double, 3> &mtl3,
    double lambda
) const
{
    using std::lrint;
    using std::floor;

    fixvec<double, 2> mtl = mtl2 / scale;

    // Pick up the frustum overloads by ADL.
    fixvec<long, 2> cell_ll = lrint(floor(mtl));

    fixvec<double, 2> lerp_param = mtl - cell_ll;
    fixed<double, 2, 2> lerp_table = {
        (1 - lerp_param(1)) * (1 - lerp_param(0)),
        (1 - lerp_param(1)) * lerp_param(0),
        lerp_param(1) * (1 - lerp_param(0)),
        lerp_param(1) * lerp_param(0)
    };

    double val = 0.0;
    for(long i = 0; i < 2; ++i)
    {
        for(long j = 0; j < 2; ++j)
        {
            fixvec<long, 2> corner_idx = {cell_ll(0) + i, cell_ll(1) + j};
            auto gradient = kernel[kernel_idx(corner_idx(0), corner_idx(1))];
            fixvec<double, 2> displacement = mtl - corner_idx;

            val += lerp_table(i,j) * iprod(displacement, gradient);
        }
    }

    return {val};
}

perlingrad2_mtlmap2::perlingrad2_mtlmap2(double scale_in, size_t seed)
    : scale(scale_in)
{
    using std::begin;
    using std::end;
    using std::generate;

    std::mt19937 g(seed);
    uniform_unitv_distribution<double, 2> dist;
    generate(begin(kernel), end(kernel), [&](){return dist(g);});
}

perlingrad2_mtlmap2::~perlingrad2_mtlmap2()
{
}

fixvec<double, 2> perlingrad2_mtlmap2::value(
    const fixvec<double, 2> &mtl2,
    const fixvec<double, 3> &mtl3,
    double lambda
) const
{
    using std::lrint;
    using std::floor;

    fixvec<double, 2> mtl = mtl2 / scale;

    // Pick up frustum overloads by ADL.
    fixvec<long, 2> cell_ll = lrint(floor(mtl));

    fixvec<double, 2> lerp_param = mtl - cell_ll;
    fixed<double, 2, 2> lerp_table = {
        (1 - lerp_param(1)) * (1 - lerp_param(0)),
        (1 - lerp_param(1)) * lerp_param(0),
        lerp_param(1) * (1 - lerp_param(0)),
        lerp_param(1) * lerp_param(0)
    };

    fixvec<double, 2> val = {0.0, 0.0};
    for(long i = 0; i < 2; ++i)
    {
        for(long j = 0; i < 2; ++j)
        {
            fixvec<long, 2> corner_idx = {cell_ll(0) + i, cell_ll(1) + j};
            auto gradient = kernel[kernel_idx(corner_idx(0), corner_idx(1))];

            val += lerp_table(i,j) * gradient;
        }
    }

    return val;
}

}
