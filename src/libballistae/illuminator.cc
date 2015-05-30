#include <libballistae/illuminator.hh>

#include <cmath>

#include <limits>

#include <libballistae/dense_signal.hh>
#include <libballistae/scene.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

dir_illuminator::~dir_illuminator()
{
}

void dir_illuminator::crush(const scene &the_scene, double time)
{
}

illumination_info dir_illuminator::power_at_point(
    const scene &the_scene,
    const fixvec<double, 3> &query_point,
    double lambda_nm
) const
{
    // Query ray.
    ray_segment<double, 3> shadow_ray = {
        {query_point, -direction},
        {epsilon<double>(), std::numeric_limits<double>::infinity()}
    };

    contact<double> contact;
    const crushed_scene_element *hit_element;

    std::tie(contact, hit_element) = scene_ray_intersect(
        the_scene,
        shadow_ray
    );

    if(contact.t == std::numeric_limits<double>::infinity())
        return {interpolate(spectrum, lambda_nm), direction};
    else
        return {0.0, {0.0, 0.0, 0.0}};
}

point_illuminator::~point_illuminator()
{
}

void point_illuminator::crush(const scene &the_scene, double time)
{
}

illumination_info point_illuminator::power_at_point(
    const scene &the_scene,
    const fixvec<double, 3> &query_point,
    double lambda_nm
) const
{
    using std::pow;

    fixvec<double, 3> direction = normalise(position - query_point);
    double distance = norm(position - query_point);
    
    // Query ray.
    ray_segment<double, 3> shadow_ray = {
        {query_point, direction},
        {0.0, distance}
    };

    contact<double> contact;
    const crushed_scene_element *hit_element;

    std::tie(contact, hit_element) = scene_ray_intersect(
        the_scene,
        shadow_ray
    );

    if(! std::isnan(contact.t))
        return {interpolate(spectrum, lambda_nm) / pow(distance, 2), direction};
    else
        return {0.0, {0.0, 0.0, 0.0}};
}

}
