#ifndef LIBBALLISTAE_UNIFORM_SPHERE_SAMPLING_HH
#define LIBBALLISTAE_UNIFORM_SPHERE_SAMPLING_HH

#include <cstddef>

#include <vector>

#include <libballistae/vector.hh>

namespace ballistae
{

template<class Field>
struct uniform_sphere_sampling
{
    std::vector<std::fixvec<Field, 3>> vecs;
};

template<class Field>
uniform_sphere_sampling gen_uniform_sphere_sampling(size_t subdiv)
{
    struct face {size_t a; size_t b; size_t c;};

    std::vector<std::fixvec<Field, 3>> verts = {
        {-1, -1, -1},
        {-1,  1,  1},
        { 1, -1,  1},
        { 1,  1, -1}
    };

    for(auto &vert : verts)
    {
        vert = arma::normalise(vert);
    }

    std::vector<face> faces = {{0,1,2}, {0,3,1}, {0,2,3}, {1,3,2}};

    for(size_t i = 0; i < subdiv
}

}

#endif
