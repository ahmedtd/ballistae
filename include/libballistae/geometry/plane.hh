#ifndef BALLISTAE_GEOMETRY_PLANE_HH
#define BALLISTAE_GEOMETRY_PLANE_HH

#include <libballistae/geometry.hh>

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/contact.hh>
#include <libballistae/ray.hh>
#include <libballistae/span.hh>

namespace ballistae
{

class plane : public geometry
{
public:
    plane();
    virtual ~plane();

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
