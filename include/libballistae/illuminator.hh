#ifndef LIBBALLISTAE_ILLUMINATOR_HH
#define LIBBALLISTAE_ILLUMINATOR_HH

////////////////////////////////////////////////////////////////////////////////
/// The simple light subsystem.
////////////////////////////////////////////////////////////////////////////////
///
/// Provides some canonical 3D-graphics light-types.  These lights can be
/// registered into a scene.  Materials that use simple lighting models (phong,
/// etc.) will hook into them.  Other materials that use ballistae's
/// power-tracing capabilities will probably not use them directly (though there
/// is no restriction to that effect).
///
/// Plugins are supported, in the same manner as geometry and materials.  The
/// most useful types of lights are defined directly in libballistae, though.

#include <random>

#include <libballistae/dense_signal.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

struct illumination_info
{
    double power;
    fixvec<double, 3> arrival;
};

struct scene;

class illuminator
{
public:
    virtual ~illuminator() {}

    virtual void crush(const scene &the_scene, double time) = 0;

    virtual illumination_info power_at_point(
        const scene &the_scene,
        const fixvec<double, 3> &query_point,
        double lambda_nm
    ) const = 0;
};

class dir_illuminator : public illuminator
{
public:
    dense_signal<double> spectrum;
    fixvec<double, 3> direction;

public:

    virtual ~dir_illuminator();

    virtual void crush(const scene &the_scene, double time);
    
    virtual illumination_info power_at_point(
        const scene &the_scene,
        const fixvec<double, 3> &query_point,
        double lambda_nm
    ) const;
};

class point_illuminator : public illuminator
{
public:
    dense_signal<double> spectrum;
    fixvec<double, 3> position;

public:

    virtual ~point_illuminator();

    virtual void crush(const scene &the_scene, double time);
    
    virtual illumination_info power_at_point(
        const scene &the_scene,
        const fixvec<double, 3> &query_point,
        double lambda_nm
    ) const;
};

}

#endif
