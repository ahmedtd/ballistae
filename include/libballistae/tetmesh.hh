#ifndef LIBBALLISTAE_TETMESH_HH
#define LIBBALLISTAE_TETMESH_HH

#include <array>
#include <vector>

#include "include/libballistae/vector.hh"

namespace ballistae {

constexpr std::array<std::array<size_t, 3>, 4> tetcell_face_indices = {
    {0, 1, 2}, {1, 0, 3}, {2, 3, 0}, {3, 2, 1}};

template <class Scalar>
using tetcell = std::array<fixvec<Scalar, 3>, 4>;

struct tetmesh {
  std::vector<tetcell> cells;
};

template <class Scalar>
struct tetcell_baked {
  std::array<fixvec<Scalar, 3>, 4> v0;
  std::array<fixvec<Scalar, 3>, 4> u;
  std::array<fixvec<Scalar, 3>, 4> v;

  std::array<fixvec<Scalar, 3>, 4> n;

  std::array<Scalar, 4> uu;
  std::array<Scalar, 4> uv;
  std::array<Scalar, 4> vv;

  std::array<Scalar, 4> recip_denom;
};

tetcell_baked bake(const tetcell &cell) {
  tetcell_baked baked;

  for (size_t i = 0; i < 4; ++i) {
    const auto &v0 = cell[tetcell_face_indices[i][0]];
    const auto &v1 = cell[tetcell_face_indices[i][1]];
    const auto &v2 = cell[tetcell_face_indices[i][2]];

    baked.v0[i] = v0;

    baked.u[i] = v1 - v0;
    baked.v[i] = v2 - v0;

    baked.n[i] = normalise(cfprod(baked.u[i], baked.v[i]));

    baked.uu[i] = iprod(baked.u[i], baked.v[i]);
    baked.uv[i] = iprod(baked.u[i], baked.v[i]);
    baked.vv[i] = iprod(baked.v[i], baked.v[i]);

    baked.recip_denom[i] =
        1.0 / (baked.uu[i] * baked.vv[i] - baked.uv[i] * baked.uv[i]);
  }

  return baked;
}

template <class Scalar>
contact<Scalar> ray_into(const tetcell_baked<Scalar> &cell,
                         const ray_segment<Scalar, 3> &query,
                         const int want_type) {
  for (size_t i = 0; i < 4; ++i) {
  }
}

}  // namespace ballistae

#endif
