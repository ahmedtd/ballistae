#ifndef LIBBALLISTAE_VECTOR_HH
#define LIBBALLISTAE_VECTOR_HH

#include <cstddef>

#include "frustum/indicial/fixed.hh"

namespace ballistae {

using namespace frustum;

template <class Field>
constexpr Field epsilon() {
  return Field(1) / Field(10000);
}

}  // namespace ballistae

#endif
