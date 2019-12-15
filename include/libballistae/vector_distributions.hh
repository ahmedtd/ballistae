#ifndef LIBBALLISTAE_UNIFORM_SPHERE_DIST_HH
#define LIBBALLISTAE_UNIFORM_SPHERE_DIST_HH

#include <cstddef>

#include <random>

#include "include/libballistae/vector.hh"

// Note that all the std:: distributions are undefined for Field types that are
// not float, double, or long double.

namespace ballistae
{

template<class Field, size_t D>
struct uniform_unitv_distribution
{
    std::uniform_real_distribution<Field> elt_dist;

    uniform_unitv_distribution()
        : elt_dist(Field(-1), Field(1))
    {
    }

    template<class Gen>
    fixvec<Field, D> operator()(Gen &g)
    {
        fixvec<Field, D> result;

        do
        {
            for(size_t i = 0; i < D; ++i)
                result(i) = elt_dist(g);
        } while(norm(result) > Field(1) || norm(result) == Field(0));

        return normalise(result);
    }
};

template<class Field, size_t D>
struct hemisphere_unitv_distribution
{
    uniform_unitv_distribution<Field, D> backing_dist;
    fixvec<Field, D> normal;

    hemisphere_unitv_distribution(const fixvec<double, 3> &normal_in)
        : normal(normal_in)
    {
    }

    template<class Gen>
    fixvec<Field, D> operator()(Gen &g)
    {
        fixvec<Field, D> candidate = backing_dist(g);
        if(iprod(candidate, normal) < Field(0))
            candidate = -candidate;
        return candidate;
    }

};

template<class Field, size_t D>
struct cosine_unitv_distribution
{
    fixvec<Field, D> normal;
    uniform_unitv_distribution<Field, D> backing_dist;
    std::uniform_real_distribution<Field> rejection_dist;

    cosine_unitv_distribution(const fixvec<Field, D> &normal_in)
        : normal(normal_in),
          backing_dist(),
          rejection_dist(Field(-1), Field(1))
    {
    }

    template<class Gen>
    fixvec<Field, D> operator()(Gen &g)
    {
        fixvec<Field, D> candidate;
        double cosine;
        do
        {
            candidate = backing_dist(g);
            cosine = iprod(normal, candidate);

            if(cosine < Field(0))
            {
                cosine = -cosine;
                candidate = -candidate;
            }
        }
        while(rejection_dist(g) < cosine);

        return candidate;
    }
};

}

#endif
