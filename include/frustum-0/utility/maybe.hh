
#ifndef FRUSTUM_UTILITY_MAYBE_HH
#define FRUSTUM_UTILITY_MAYBE_HH

#include <boost/variant.hpp>
#include <system_error>

namespace frustum {

template <class Wrapped>
using maybe = boost::variant<std::error_code, Wrapped>;

template <class Wrapped>
std::error_code error(const maybe<Wrapped> &mw) {
  const std::error_code *maybe_error = boost::get<std::error_code>(&mw);
  if (maybe_error)
    return *maybe_error;
  else
    return std::error_code(0, std::generic_category());
}

template <class Wrapped>
Wrapped &value(maybe<Wrapped> &mw) {
  Wrapped *maybe_wrapped = boost::get<Wrapped>(&mw);
#ifndef FRUSTUM_DISABLE_CHECKS
  if (maybe_wrapped)
    return *maybe_wrapped;
  else
    throw std::logic_error("Unchecked access to value in frustum::maybe.");
#else
  return *maybe_wrapped;
#endif
}

template <class Wrapped>
const Wrapped &value(const maybe<Wrapped> &mw) {
  const Wrapped *maybe_wrapped = boost::get<Wrapped>(&mw);
#ifndef FRUSTUM_DISABLE_CHECKS
  if (maybe_wrapped)
    return *maybe_wrapped;
  else
    throw std::logic_error("Unchecked access to value in frustum::maybe.");
#else
  return *maybe_wrapped;
#endif
}

}  // namespace frustum

#endif
