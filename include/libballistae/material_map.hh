#ifndef LIBBALLISTAE_MATERIAL_MAP_HH
#define LIBBALLISTAE_MATERIAL_MAP_HH

#include <cstddef>

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/dense_signal.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

template<size_t D>
class mtlmap
{
public:
    virtual ~mtlmap() {};

    virtual fixvec<double, D> value(
        const fixvec<double, 2> &mtl2,
        const fixvec<double, 3> &mtl3,
        double lambda
    ) const = 0;
};

class constant_mtlmap1 : public mtlmap<1>
{
 public:
    dense_signal<double> spectrum;

    constant_mtlmap1(dense_signal<double> spectrum_in);
    virtual ~constant_mtlmap1();

    virtual fixvec<double, 1> value(
        const fixvec<double, 2> &mtl2,
        const fixvec<double, 3> &mtl3,
        double lambda
    ) const;
};

class checkerboard_mtlmap1 : public mtlmap<1>
{
public:
    const mtlmap<1> *even_mtlmap;
    const mtlmap<1> *odd_mtlmap;

    double period;
    bool volumetric;
    
    checkerboard_mtlmap1(
        const mtlmap<1> *even_mtlmap_in,
        const mtlmap<1> *odd_mtlmap_in,
        double period_in,
        bool volumetric_in
    );
    
    virtual ~checkerboard_mtlmap1();

    virtual fixvec<double, 1> value(
        const fixvec<double, 2> &mtl2,
        const fixvec<double, 3> &mtl3,
        double lambda
    ) const;
};

class perlinval2_mtlmap1 : public mtlmap<1>
{
public:
    double scale;
    std::array<fixvec<double, 2>, 256*256> kernel;

    perlinval2_mtlmap1(double scale_in, size_t seed);
    virtual ~perlinval2_mtlmap1();

    virtual fixvec<double, 1> value(
        const fixvec<double, 2> &mtl2,
        const fixvec<double, 3> &mtl3,
        double lambda
    ) const;
};

class perlingrad2_mtlmap2 : public mtlmap<2>
{
public:
    double scale;
    std::array<fixvec<double, 2>, 256*256> kernel;

    perlingrad2_mtlmap2(double scale_in, size_t seed);
    virtual ~perlingrad2_mtlmap2();

    virtual fixvec<double, 2> value(
        const fixvec<double, 2> &mtl2,
        const fixvec<double, 3> &mtl3,
        double lambda
    ) const;
};

}

#endif
