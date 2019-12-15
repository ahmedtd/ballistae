#ifndef FRUSTUM_MATRIX_TABLE_HH
#define FRUSTUM_MATRIX_TABLE_HH

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <tuple>
#include <type_traits>

#include "frustum/indicial/address.hh"
#include "frustum/indicial/generic_ops.hh"
#include "frustum/utility/compile_time_ops.hh"
#include "frustum/utility/tag_structs.hh"

namespace frustum {

/// Dense, fixed-dimension indicial implementation
template <class Entry, std::size_t... Dims>
struct fixed {
  using entry_type = Entry;

  constexpr static size_t s_num_entries = ct_ops::product(Dims...);

  std::array<Entry, s_num_entries> container;

  /// Make a zero-filled indicial.
  static fixed<Entry, Dims...> zero() {
    fixed<Entry, Dims...> result;
    for (size_t i = 0; i < s_num_entries; ++i) result.container[i] = Entry(0);
    return result;
  }

  /// Make an identity matrix.
  ///
  /// Right now, it's exposed only for square matrices (using enable_if).
  ///
  /// The dance with the UnusedTag template parameter can be ignored --- it's
  /// here because enable_if requires its first argument to be dependent on a
  /// template parameter in order to work properly.  If we don't use it, then
  /// compilation will fail as soon as we instantiate a fixed-indicial for
  /// which the test fails.
  template <class UnusedTag = unused_tag>
  static typename std::enable_if<
      ct_ops::true_helper<UnusedTag>::value && sizeof...(Dims) == 2 &&
          ct_ops::select_ith(0, Dims...) == ct_ops::select_ith(1, Dims...),
      fixed<Entry, Dims...> >::type
  eye(UnusedTag tag = (unused_tag())) {
    auto result = fixed<Entry, Dims...>::zero();
    for (size_t i = 0; i < ct_ops::select_ith(0, Dims...); ++i)
      for (size_t j = 0; j < ct_ops::select_ith(1, Dims...); ++j)
        if (i == j) result(i, j) = 1.0;
    return result;
  }

  std::size_t order() const { return sizeof...(Dims); }

  std::size_t n_entries() const { return s_num_entries; }

  std::size_t index_lim(size_t i) const {
    return ct_ops::select_ith<Dims...>(i);
  }

  template <class... IndexTypes>
  Entry &operator()(IndexTypes... indices) {
    std::size_t index =
        fixed_dimension_address(indices..., (list_end_tag()), Dims...);
    return container[index];
  }

  template <class... IndexTypes>
  const Entry &operator()(IndexTypes... indices) const {
    return const_cast<fixed<Entry, Dims...> &>(*this)(indices...);
  }

  Entry &operator[](size_t i) { return container[i]; }

  const Entry &operator[](size_t i) const {
    return const_cast<fixed<Entry, Dims...> &>(*this)[i];
  }

  template <class OEntry>
  auto operator=(const fixed<OEntry, Dims...> &o) {
    container = o.container;
    return *this;
  }

  template <class OEntry>
  auto operator+=(const fixed<OEntry, Dims...> &o) {
    for (size_t i = 0; i < s_num_entries; ++i) container[i] += o.container[i];
    return *this;
  }

  template <class OEntry>
  auto operator-=(const fixed<OEntry, Dims...> &o) {
    for (size_t i = 0; i < s_num_entries; ++i) container[i] += o.container[i];
    return *this;
  }

  template <class Scalar>
  auto operator*=(Scalar scalar) {
    for (size_t i = 0; i < s_num_entries; ++i)
      container[i] = container[i] * scalar;
    return *this;
  }

  template <class Scalar>
  auto operator/=(Scalar scalar) {
    for (size_t i = 0; i < s_num_entries; ++i)
      container[i] = container[i] / scalar;
  }

  template <class Scalar>
  auto operator%=(Scalar scalar) {
    for (size_t i = 0; i < s_num_entries; ++i)
      container[i] = container[i] % scalar;
  }
};

template <class EntryA, class EntryB, size_t... Dims>
bool operator==(const fixed<EntryA, Dims...> &a,
                const fixed<EntryB, Dims...> &b) {
  return a.container == b.container;
}

template <class EntryA, class EntryB, size_t... Dims>
bool operator!=(const fixed<EntryA, Dims...> &a,
                const fixed<EntryB, Dims...> &b) {
  return a.container != b.container;
}

////////////////////////////////////////////////////////////////////////////////
/// Scalar operations
////////////////////////////////////////////////////////////////////////////////

template <class Elt, size_t... Dims, class Scalar>
auto operator*(const fixed<Elt, Dims...> &a, Scalar s) {
  fixed<decltype(a[0] * s), Dims...> result;
  for (size_t i = 0; i < a.n_entries(); ++i) {
    result[i] = a[i] * s;
  }

  return result;
}

template <class Elt, size_t... Dims, class Scalar>
auto operator*(Scalar s, const fixed<Elt, Dims...> &a) {
  fixed<decltype(s * a[0]), Dims...> result;
  for (size_t i = 0; i < a.n_entries(); ++i) {
    result[i] = s * a[i];
  }

  return result;
}

template <class Elt, size_t... Dims, class Scalar>
auto operator/(const fixed<Elt, Dims...> &a, Scalar s) {
  fixed<decltype(a[0] / s), Dims...> result;
  for (size_t i = 0; i < a.n_entries(); ++i) {
    result[i] = a[i] / s;
  }

  return result;
}

template <class Elt, size_t... Dims, class Scalar>
auto operator%(const fixed<Elt, Dims...> &a, Scalar s) {
  fixed<decltype(a[0] % s), Dims...> result;
  for (size_t i = 0; i < a.n_entries(); ++i) {
    result[i] = a[i] % s;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// Elementwise operations.
////////////////////////////////////////////////////////////////////////////////

template <size_t... Dims, class Op, class Elt0, class... EltTail>
auto fixed_elementwise_op(Op op, const fixed<Elt0, Dims...> &arg0,
                          const fixed<EltTail, Dims...> &... argtail) {
  using EltR = decltype(op(arg0[0], argtail[0]...));

  fixed<EltR, Dims...> result;
  generic_elementwise_op(result, op, arg0, argtail...);
  return result;
}

template <class Elt, size_t... Dims>
auto operator-(const fixed<Elt, Dims...> &a) {
  return fixed_elementwise_op([](Elt e) { return -e; }, a);
}

template <class EltA, class EltB, size_t... Dims>
auto operator+(const fixed<EltA, Dims...> &a, const fixed<EltB, Dims...> &b) {
  return fixed_elementwise_op([](EltA a, EltB b) { return a + b; }, a, b);
}

template <class EltA, class EltB, size_t... Dims>
auto operator-(const fixed<EltA, Dims...> &a, const fixed<EltB, Dims...> &b) {
  return fixed_elementwise_op([](EltA a, EltB b) { return a - b; }, a, b);
}

template <class EltA, class EltB, size_t... Dims>
auto eltwise_mul(const fixed<EltA, Dims...> &a, const fixed<EltB, Dims...> &b) {
  return fixed_elementwise_op([](EltA a, EltB b) { return a * b; }, a, b);
}

template <class EltA, class EltB, size_t... Dims>
auto eltwise_div(const fixed<EltA, Dims...> &a, const fixed<EltB, Dims...> &b) {
  return fixed_elementwise_op([](EltA a, EltB b) { return a / b; }, a, b);
}

template <class Elt, size_t... Dims>
auto floor(const fixed<Elt, Dims...> &a) {
  using std::floor;

  return fixed_elementwise_op([](Elt e) { return floor(e); }, a);
}

template <class Elt, size_t... Dims>
auto lrint(const fixed<Elt, Dims...> &a) {
  using std::lrint;
  return fixed_elementwise_op([](Elt e) { return lrint(e); }, a);
}

////////////////////////////////////////////////////////////////////////////////
/// Vector operations (Linear Algebraic and Geometric).
////////////////////////////////////////////////////////////////////////////////

template <class EltA, class EltB, size_t D>
auto iprod(const fixed<EltA, D> &a, const fixed<EltB, D> &b) {
  using EltR = decltype(a(0) * b(0) + a(1) * b(1));

  EltR accum = EltR(0);
  for (size_t i = 0; i < D; ++i) accum += a(i) * b(i);
  return accum;
}

template <class EltA, class EltB>
auto cprod(const fixed<EltA, 3> &a, const fixed<EltB, 3> &b) {
  using EltR = decltype(a(1) * b(2) - a(2) * b(1));

  fixed<EltR, 3> result = {a(1) * b(2) - a(2) * b(1), a(2) * b(0) - a(0) * b(2),
                           a(0) * b(1) - a(1) * b(0)};

  return result;
}

template <class EltA, size_t D, class Power>
auto norm(const fixed<EltA, D> &a, Power p) {
  using std::pow;
  using Accum = decltype(pow(a(0), p) + pow(a(1), p));

  Accum accum = Accum(0);
  for (size_t i = 0; i < D; ++i) accum += pow(a(i), p);

  return pow(accum, Power(1) / p);
}

template <class EltA, size_t D>
auto norm(const fixed<EltA, D> &a) {
  using std::sqrt;
  return sqrt(iprod(a, a));
}

template <class EltA, size_t D, class Power>
auto normalise(const fixed<EltA, D> &a, Power p) {
  auto n = norm(a, p);
  return a / n;
}

template <class EltA, size_t D>
auto normalise(const fixed<EltA, D> &a) {
  auto n = norm(a);
  return a / n;
}

template <class EltA, class EltB, size_t D>
auto reject(const fixed<EltA, D> &a, const fixed<EltB, D> &b) {
  return b - normalise(a) * (iprod(a, b) / norm(a));
}

template <class EltA, class EltB, size_t D>
auto reflect(const fixed<EltA, D> &a, const fixed<EltB, D> &n) {
  return a - EltA(2) * iprod(a, n) * n;
}

////////////////////////////////////////////////////////////////////////////////
/// Matrix operations.
////////////////////////////////////////////////////////////////////////////////

/// Transposition.
template <class Elt, size_t R, size_t C>
auto transpose(const fixed<Elt, R, C> &a) {
  fixed<Elt, C, R> transpose;
  for (size_t r = 0; r < R; ++r)
    for (size_t c = 0; c < C; ++c) transpose(c, r) = a(r, c);

  return transpose;
}

/// Matrix-Matrix multiplication.
template <class EltA, class EltB, size_t R, size_t S, size_t C>
auto operator*(const fixed<EltA, R, S> &a, const fixed<EltB, S, C> &b) {
  using EltR = decltype(a(0, 0) * b(0, 0) + a(0, 1) * b(1, 0));
  auto result = fixed<EltR, R, C>::zero();

  for (size_t i = 0; i < R; ++i)
    for (size_t swizzle = 0; swizzle < S; ++swizzle)
      for (size_t j = 0; j < C; ++j)
        result(i, j) += a(i, swizzle) * b(swizzle, j);

  return result;
}

/// Matrix-Vector multiplication.
template <class EltA, class EltB, size_t R, size_t S>
auto operator*(const fixed<EltA, R, S> &a, const fixed<EltB, S> &b) {
  using EltR = decltype(a(0, 0) * b(0) + a(0, 1) * b(1));
  auto result = fixed<EltR, R>::zero();

  for (size_t i = 0; i < R; ++i)
    for (size_t swizzle = 0; swizzle < S; ++swizzle)
      result(i) += a(i, swizzle) * b(swizzle);

  return result;
}

template <class Elt, size_t R, size_t C, size_t A>
void rowechelon_inplace(fixed<Elt, R, C> &m, fixed<Elt, R, A> &a) {
  using std::abs;
  using std::min;
  using std::swap;

  for (size_t k = 0; k < min(R, C); ++k) {
    // Select row with best pivot.
    size_t maxrow = k;
    for (size_t i = k; i < R; ++i)
      if (abs(m(i, k)) > abs(m(maxrow, k))) maxrow = i;

    // Swap selected row to current row.
    if (maxrow != k) {
      for (size_t i = 0; i < C; ++i) swap(m(k, i), m(maxrow, i));

      for (size_t i = 0; i < A; ++i) swap(a(k, i), a(maxrow, i));
    }

    // Now the pivot element is at m(k,k).
    auto &pivot = m(k, k);

    for (size_t r = k + 1; r < R; ++r) {
      auto scale = m(r, k) / pivot;

      for (size_t c = k + 1; c < C; ++c) {
        m(r, c) -= m(k, c) * scale;
      }

      for (size_t c = 0; c < A; ++c) {
        a(r, c) -= a(k, c) * scale;
      }

      m(r, k) = Elt(0);
    }
  }
}

template <class Elt, size_t R, size_t C, size_t A>
void backsub_inplace(fixed<Elt, R, C> &m, fixed<Elt, R, A> &a) {
  using std::min;

  for (size_t k = min(R, C) - 1; k > 0; --k) {
    // m(k,k) is the pivot.

    // Nullify all entries above the pivot element.
    for (size_t r = 0; r < k; ++r) {
      auto scale = m(r, k) / m(k, k);

      m(r, k) = Elt(0);
      for (size_t c = k + 1; c < C; ++c) {
        m(r, c) -= scale * m(k, c);
      }

      // Mirror the action in the augmented matrix.
      for (size_t c = 0; c < A; ++c) {
        a(r, c) -= scale * a(k, c);
      }
    }
  }

  // Now we simply need to divide each row by its pivot.
  for (size_t k = 0; k < min(R, C); ++k) {
    for (size_t c = k + 1; c < C; ++c) m(k, c) /= m(k, k);
    for (size_t c = 0; c < A; ++c) a(k, c) /= m(k, k);
    m(k, k) = Elt(1);
  }
}

template <class Elt, size_t D, size_t A>
void solve_inplace(fixed<Elt, D, D> &a, fixed<Elt, D, A> &b) {
  rowechelon_inplace(a, b);
  backsub_inplace(a, b);
}

template <class Elt, size_t D, size_t A>
auto solve(const fixed<Elt, D, D> &a, const fixed<Elt, D, A> &b) {
  auto a_r = a;
  auto b_r = b;

  solve_inplace(a_r, b_r);

  return b_r;
}

template <class Elt, size_t D>
auto inverse(const fixed<Elt, D, D> &a) {
  auto a_r = a;
  auto b_r = fixed<Elt, D, D>::eye();
  solve_inplace(a_r, b_r);
  return b_r;
}

template <class Elt, size_t D>
using fixvec = fixed<Elt, D>;

template <class Elt, size_t R, size_t C>
using fixmat = fixed<Elt, R, C>;

}  // namespace frustum

#endif
