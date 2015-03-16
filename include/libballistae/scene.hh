#ifndef LIBBALLISTAE_SCENE_HH
#define LIBBALLISTAE_SCENE_HH

#include <memory>
#include <vector>

namespace ballistae
{

class geom_priv;
class matr_priv;

struct scene
{
    std::vector<std::shared_ptr<matr_priv>> materials;
    std::vector<std::shared_ptr<geom_priv>> geometries;
};

}

#endif
