#ifndef LIBBALLISTAE_UNIFORM_SPHERE_DIST_HH
#define LIBBALLISTAE_UNIFORM_SPHERE_DIST_HH

#include <cstddef>

#include <random>

#include <libballistae/vector.hh>

// Note that all the std:: distributions are undefined for Field types that are
// not float, double, or long double.

namespace ballistae
{

template<class Field, size_t D>
struct uniform_sphere_dist
{
    Field radius;

    std::uniform_real_distribution<Field> elt_dist;

    uniform_sphere_dist(Field radius);

    template<class Gen>
    fixvec<Field, D> operator()(Gen &g);
};

template<class Field, size_t D>
uniform_sphere_dist<Field, D>::uniform_sphere_dist(Field radius_in)
    : radius(radius_in),
      elt_dist(Field(-1), Field(1))
{
}

template<class Field, size_t D>
template<class Gen>
fixvec<Field, D> uniform_sphere_dist<Field, D>::operator()(Gen &g)
{
    fixvec<Field, D> result;

    do
    {
        result = {elt_dist(g), elt_dist(g), elt_dist(g)};
    } while(arma::norm(result) > Field(1) || arma::norm(result) == Field(0));

    return radius * arma::normalise(result);
}

template<class Field, size_t D>
struct uniform_hemisphere_dist
{
    uniform_sphere_dist<Field, D> backing_dist;
    fixvec<Field, D> normal;

    uniform_hemisphere_dist(Field radius_in, fixvec<Field, D> normal_in);

    template<class Gen>
    fixvec<Field, D> operator()(Gen &g);
};

template<class Field, size_t D>
uniform_hemisphere_dist<Field, D>::uniform_hemisphere_dist(
    Field radius_in,
    fixvec<Field, D> normal_in
)
    : backing_dist(radius_in),
      normal(arma::normalise(normal_in))
{
}

template<class Field, size_t D>
template<class Gen>
fixvec<Field, D> uniform_hemisphere_dist<Field, D>::operator()(Gen &g)
{
    fixvec<Field, D> result = backing_dist(g);
    Field proj_len = arma::dot(result, normal);

    if(proj_len < Field(0))
    {
        result -= Field(2) * proj_len * normal;
    }

    return result;
}

}

#endif
