#ifndef LIBBALLISTAE_SPECTRUM_HH
#define LIBBALLISTAE_SPECTRUM_HH

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "libballistae/dense_signal.hh"

namespace ballistae {

struct srgb_tag {};
struct xyz_tag {};

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

template <class Elem, class CSTag>
color3<Elem, CSTag> clamp_1(const color3<Elem, CSTag> &a) {
  using std::min;
  color3<Elem, CSTag> result = {min(Elem(1), a.channels[0]),
                                min(Elem(1), a.channels[1]),
                                min(Elem(1), a.channels[2])};
  return result;
}

template <class Elem, class CSTag>
color3<Elem, CSTag> clamp_01(const color3<Elem, CSTag> &a) {
  return clamp_0<Elem, CSTag>(clamp_1<Elem, CSTag>(a));
}

color3<float, srgb_tag> xyz_to_srgb(const color3<float, xyz_tag> &src);
color3<float, xyz_tag> srgb_to_xyz(const color3<float, srgb_tag> &src);

using color_d_srgb = color3<double, srgb_tag>;
using color_d_XYZ = color3<double, xyz_tag>;

template <class Field>
color3<Field, xyz_tag> spectral_to_XYZ(const Field &src_x, const Field &lim_x,
                                       const Field &y) {
  color3<Field, xyz_tag> result;
  result.channels[0] = partial_iprod(cie_2006_X<Field>(), src_x, lim_x, y);
  result.channels[1] = partial_iprod(cie_2006_Y<Field>(), src_x, lim_x, y);
  result.channels[2] = partial_iprod(cie_2006_Z<Field>(), src_x, lim_x, y);
  return result;
}

template <class Field>
dense_signal<Field> srgb_to_spectral(const color3<Field, srgb_tag> &src) {
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
