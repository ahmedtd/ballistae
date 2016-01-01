#ifndef BALLISTAE_GEOMETRY_CYLINDER_HH
#define BALLISTAE_GEOMETRY_CYLINDER_HH

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/contact.hh>
#include <libballistae/geometry.hh>
#include <libballistae/ray.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

class cylinder : public geometry
{
public:
    fixvec<double, 3> center;
    fixvec<double, 3> axis;
    double radius_squared;

public:
    cylinder(
        const fixvec<double, 3> &center_in,
        const fixvec<double, 3> &axis_in,
        const double &radius_in
    );

    virtual ~cylinder();

    virtual aabox<double, 3> get_aabox();

    virtual void crush(double time);

    virtual contact<double> ray_into(
        const ray_segment<double,3> &query
    ) const;

    virtual contact<double> ray_exit(
        const ray_segment<double,3> &query
    ) const;
};

}

#endif
