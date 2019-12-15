
#ifndef FRUSTUM_UTILITY_TYPE_SUPPORT_HH
#define FRUSTUM_UTILITY_TYPE_SUPPORT_HH

#include <iterator>
#include <tuple>

namespace frustum {

template <class Iterator>
using range = std::tuple<Iterator, Iterator>;

template <class Iterator>
Iterator begin(const range<Iterator> &the_range) {
  return std::get<0>(the_range);
}

template <class Iterator>
Iterator end(const range<Iterator> &the_range) {
  return std::get<1>(the_range);
}

template <class TypeA, class TypeB>
struct product {
  auto exemplar(TypeA a, TypeB b) -> decltype(a * b) { return a * b; }

  typedef decltype(exemplar((TypeA()), (TypeB()))) value;
};

}  // namespace frustum

#endif
