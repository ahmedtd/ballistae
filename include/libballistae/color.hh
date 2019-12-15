#ifndef LIBBALLISTAE_SPECTRUM_HH
#define LIBBALLISTAE_SPECTRUM_HH

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "include/libballistae/dense_signal.hh"

namespace ballistae {

struct rgb_tag {};
struct XYZ_tag {};

template <class Elem, class CSTag>
struct color3 {
  std::array<Elem, 3> channels;

  color3<Elem, CSTag> &operator+=(const color3<Elem, CSTag> &other) {
    for (std::size_t i = 0; i < this->channels.size(); ++i) {
      this->channels[i] += other.channels[i];
    }
    return *this;
  }

  Elem &operator[](std::size_t i) { return channels[i]; }

  const Elem &operator[](std::size_t i) const { return channels[i]; }
};

template <class Elem, class CSTag>
color3<Elem, CSTag> operator*(const Elem &s, const color3<Elem, CSTag> &c) {
  auto r = c;
  for (std::size_t i = 0; i < c.channels.size(); ++i) {
    r.channels[i] = s * c.channels[i];
  }
  return r;
}

template <class Elem, class CSTag>
color3<Elem, CSTag> operator+(const color3<Elem, CSTag> &a,
                              const color3<Elem, CSTag> &b) {
  auto r = a;
  r += b;
  return r;
}

template <class Elem, class CSTag>
color3<Elem, CSTag> clamp_0(const color3<Elem, CSTag> &a) {
  using std::max;

  color3<Elem, CSTag> result = {max(Elem(0), a.channels[0]),
                                max(Elem(0), a.channels[1]),
                                max(Elem(0), a.channels[2])};
  return result;
}

template <class Field>
color3<Field, rgb_tag> to_rgb(const color3<Field, XYZ_tag> &src) {
  color3<Field, rgb_tag> result = {
      3.240479 * src[0] + -1.537150 * src[1] + -0.498535 * src[2],
      -0.969265 * src[0] + 1.875992 * src[1] + 0.041556 * src[2],
      0.055648 * src[0] + -0.204043 * src[1] + 1.057311 * src[2]};
  return result;
}

using color_d_rgb = color3<double, rgb_tag>;
using color_d_XYZ = color3<double, XYZ_tag>;

template <class Field>
color3<Field, XYZ_tag> spectral_to_XYZ(const Field &src_x, const Field &lim_x,
                                       const Field &y) {
  color3<Field, XYZ_tag> result;
  result.channels[0] = partial_iprod(cie_2006_X<Field>(), src_x, lim_x, y);
  result.channels[1] = partial_iprod(cie_2006_Y<Field>(), src_x, lim_x, y);
  result.channels[2] = partial_iprod(cie_2006_Z<Field>(), src_x, lim_x, y);
  return result;
}

template <class Field>
dense_signal<Field> rgb_to_spectral(const color3<Field, rgb_tag> &src) {
  const Field &r = src[0];
  const Field &g = src[1];
  const Field &b = src[2];

  dense_signal<Field> result = smits_zero<Field>();

  if (r < g && r < b) {
    result += r * smits_white<Field>();
    if (g < b) {
      result += (g - r) * smits_cyan<Field>();
      result += (b - g) * smits_blue<Field>();
    } else {
      result += (b - r) * smits_cyan<Field>();
      result += (g - b) * smits_green<Field>();
    }
  } else if (g < r && g < b) {
    result += g * smits_white<Field>();
    if (r < b) {
      result += (r - g) * smits_magenta<Field>();
      result += (b - r) * smits_blue<Field>();
    } else {
      result += (b - g) * smits_magenta<Field>();
      result += (r - b) * smits_red<Field>();
    }
  } else {
    result += b * smits_white<Field>();
    if (r < g) {
      result += (r - b) * smits_yellow<Field>();
      result += (g - r) * smits_green<Field>();
    } else {
      result += (g - b) * smits_yellow<Field>();
      result += (r - g) * smits_red<Field>();
    }
  }

  return result;
}

}  // namespace ballistae

#endif
