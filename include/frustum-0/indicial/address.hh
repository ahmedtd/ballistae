#ifndef FRUSTUM_0_INDICIAL_ADDRESS_HH
#define FRUSTUM_0_INDICIAL_ADDRESS_HH

#include <array>
#include <cassert>
#include <iterator>

#include "include/frustum-0/utility/compile_time_ops.hh"
#include "include/frustum-0/utility/tag_structs.hh"

namespace frustum {

template <class IdxHead, class DimHead>
constexpr size_t fixed_dimension_address(IdxHead idxhead, list_end_tag unused,
                                         DimHead dimhead) {
  return idxhead;
}

template <class IdxHead, class IdxSecond, class... IdxTail, class DimHead,
          class DimSecond, class... DimTail>
constexpr size_t fixed_dimension_address(IdxHead idxhead, IdxSecond idxsecond,
                                         IdxTail... idxtail,
                                         list_end_tag unused, DimHead dimhead,
                                         DimSecond dimsecond,
                                         DimTail... dimtail) {
  return idxhead * ct_ops::product(dimsecond, dimtail...) +
         fixed_dimension_address(idxsecond, idxtail..., unused, dimsecond,
                                 dimtail...);
}

///
template <class IndexIterator, class WidthIterator>
size_t address(IndexIterator index_beg, IndexIterator index_end,
               WidthIterator width_beg, WidthIterator width_end) {
  assert(std::distance(index_beg, index_end) ==
         std::distance(width_beg, width_end));

  auto index_rbeg = std::reverse_iterator<IndexIterator>(index_beg);
  auto index_rend = std::reverse_iterator<IndexIterator>(index_end);
  auto width_rbeg = std::reverse_iterator<WidthIterator>(width_beg);
  auto width_rend = std::reverse_iterator<WidthIterator>(width_end);

  std::size_t computed_index = 0;
  std::size_t lower_size = 1;
  for (; index_rbeg != index_rend; ++index_rbeg, ++width_rbeg) {
    computed_index += (*index_rbeg) * lower_size;
    lower_size *= *width_rbeg;
  }

  return computed_index;
}

template <class Iterator>
constexpr size_t container_size(Iterator src, Iterator lim) {
  return (src == lim) ? 1 : *src * container_size(src + 1, lim);
}

}  // namespace frustum

#endif
