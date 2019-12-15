#ifndef LIBBALLISTAE_IMAGE_PATCH_HH
#define LIBBALLISTAE_IMAGE_PATCH_HH

#include <vector>

namespace balliste {

// Instead of keeping the actual basis vectors, just keep their
// indices in the lexical ordering of all basis vectors.

template <class Elem>
struct image_patch {
  // Cosine basis representation
  std::size_t n_row_components;
  std::size_t n_col_components;
  std::size_t n_frq_components;
  std::vector<Elem> basis_vectors;
};

}  // namespace balliste

#endif
