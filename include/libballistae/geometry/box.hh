#ifndef BALLISTAE_GEOMETRY_BOX_HH
#define BALLISTAE_GEOMETRY_BOX_HH

#include <array>

#include <libballistae/contact.hh>
#include <libballistae/geometry.hh>
#include <libballistae/ray.hh>
#include <libballistae/scene.hh>
#include <libballistae/span.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

class box : public geometry
{
public:
    std::array<span<double>, 3> spans;

    box(std::array<span<double>, 3> spans_in);
    virtual ~box();

    virtual aabox<double, 3> get_aabox();
    virtual void crush(const scene &the_scene, double time);

    virtual contact<double> ray_into(
        const scene &the_scene,
        const ray_segment<double, 3> &query
    ) const;

    virtual contact<double> ray_exit(
        const scene &the_scene,
        const ray_segment<double, 3> &query
    ) const;
};

}

#endif
