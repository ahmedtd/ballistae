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

class lerp_mtlmap1 : public mtlmap<1>
{
public:
    double t_lo;
    double t_hi;
    
    const mtlmap<1> *t;
    const mtlmap<1> *a;
    const mtlmap<1> *b;

    lerp_mtlmap1(
        double t_lo_in,
        double t_hi_in,
        const mtlmap<1> *t_in,
        const mtlmap<1> *a_in,
        const mtlmap<1> *b_in
    );
    virtual ~lerp_mtlmap1();

    virtual fixvec<double, 1> value(
        const fixvec<double, 2> &mtl2,
        const fixvec<double, 3> &mtl3,
        double lambda
    ) const;
};

class level_mtlmap1 : public mtlmap<1>
{
public:
    double t_lo;
    double t_hi;

    const mtlmap<1> *t;
    const mtlmap<1> *a;
    const mtlmap<1> *b;
    
    level_mtlmap1(
        double t_lo_in,
        double t_hi_in,
        const mtlmap<1> *t_in,
        const mtlmap<1> *a_in,
        const mtlmap<1> *b_in
    );
    virtual ~level_mtlmap1();

    virtual fixvec<double, 1> value(
        const fixvec<double, 2> &mtl2,
        const fixvec<double, 3> &mtl3,
        double lambda
    ) const;
};

class checkerboard_mtlmap1 : public mtlmap<1>
{
public:
    double period;
    bool volumetric;

    checkerboard_mtlmap1(
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

class bullseye_mtlmap1 : public mtlmap<1>
{
public:
    double period;
    bool volumetric;

    bullseye_mtlmap1(
        double period_in,
        bool volumetric_in
    );

    virtual ~bullseye_mtlmap1();

    virtual fixvec<double, 1> value(
        const fixvec<double, 2> &mtl2,
        const fixvec<double, 3> &mtl3,
        double lambda
    ) const;
};

class perlinval_mtlmap1 : public mtlmap<1>
{
public:
    bool volumetric;
    double period;

    perlinval_mtlmap1(
        bool volumetric_in,
        double period_in
    );
    virtual ~perlinval_mtlmap1();

    virtual fixvec<double, 1> value(
        const fixvec<double, 2> &mtl2,
        const fixvec<double, 3> &mtl3,
        double lambda
    ) const;
};

}

#endif
