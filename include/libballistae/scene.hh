#ifndef LIBBALLISTAE_SCENE_HH
#define LIBBALLISTAE_SCENE_HH

#include <memory>
#include <vector>

#include <libballistae/affine_transform.hh>
#include <libballistae/vector.hh>

namespace ballistae
{

class geom_priv;
class matr_priv;

struct scene
{
    std::vector<std::shared_ptr<matr_priv>> materials;
    std::vector<std::shared_ptr<geom_priv>> geometries;

    std::vector<affine_transform<double, 3>> trans_for;
    std::vector<affine_transform<double, 3>> trans_inv;
};

}

#endif
