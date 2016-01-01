#ifndef BALLISTAE_GEOMETRY_SPHERE_HH
#define BALLISTAE_GEOMETRY_SPHERE_HH

#include <libballistae/geometry.hh>

#include <libballistae/ray.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

class sphere : public geometry
{
public:
    sphere();

    virtual ~sphere();

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
