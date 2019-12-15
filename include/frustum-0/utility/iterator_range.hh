
#ifndef FRUSTUM_0_UTILITY_ITERATOR_RANGE_HH
#define FRUSTUM_0_UTILITY_ITERATOR_RANGE_HH

namespace frustum {

template <class Iterator>
struct iterator_range {
  Iterator begin;
  Iterator end;
};

template <class Iterator>
iterator_range<Iterator> make_iterator_range(Iterator &begin, Iterator &end) {
  return (iterator_range<Iterator>(begin, end));
}

template <class Iterator>
Iterator begin(iterator_range<Iterator> &the_range) {
  return the_range.begin;
}

template <class Iterator>
Iterator end(iterator_range<Iterator> &the_range) {
  return the_range.end;
}

template <class BidiIterator>
auto reverse(iterator_range<BidiIterator> &the_range) {
  std::reverse_iterator<BidiIterator> rbegin(the_range.begin);
  std::reverse_iterator<BidiIterator> rend(the_range.end);

  return make_iterator_range(rbegin, rend);
}

}  // namespace frustum

#endif
