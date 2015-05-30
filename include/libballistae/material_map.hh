#ifndef LIBBALLISTAE_MATERIAL_MAP_HH
#define LIBBALLISTAE_MATERIAL_MAP_HH

#include <cstddef>

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/dense_signal.hh>
#include <libballistae/scene.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

template<size_t D>
class mtlmap
{
public:
    virtual ~mtlmap() {};

    virtual void crush(const scene &the_scene, double time) = 0;

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

    constant_mtlmap1(const dense_signal<double> spectrum_in);
    virtual ~constant_mtlmap1();

    virtual void crush(const scene &the_scene, double time);

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

    size_t t_ind;
    size_t a_ind;
    size_t b_ind;
    
    mtlmap<1> *t;
    mtlmap<1> *a;
    mtlmap<1> *b;

    lerp_mtlmap1(
        double t_lo_in,
        double t_hi_in,
        mtlmap<1> *t_in,
        mtlmap<1> *a_in,
        mtlmap<1> *b_in
    );
    virtual ~lerp_mtlmap1();

    virtual void crush(const scene &the_scene, double time);
    
    virtual fixvec<double, 1> value(
        const fixvec<double, 2> &mtl2,
        const fixvec<double, 3> &mtl3,
        double lambda
    ) const;
};

class level_mtlmap1 : public mtlmap<1>
{
public:
    double t_switch;

    size_t t_ind;
    size_t a_ind;
    size_t b_ind;
    
    mtlmap<1> *t;
    mtlmap<1> *a;
    mtlmap<1> *b;

    level_mtlmap1(
        double t_switch_in,
        mtlmap<1> *t_in,
        mtlmap<1> *a_in,
        mtlmap<1> *b_in
    );
    virtual ~level_mtlmap1();

    virtual void crush(const scene &the_scene, double time);

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

    virtual void crush(const scene &the_scene, double time);
    
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

    virtual void crush(const scene &the_scene, double time);

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

    virtual void crush(const scene &the_scene, double time);

    virtual fixvec<double, 1> value(
        const fixvec<double, 2> &mtl2,
        const fixvec<double, 3> &mtl3,
        double lambda
    ) const;
};

}

#endif
