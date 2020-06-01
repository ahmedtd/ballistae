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

inline color3<float, xyz_tag> spectral_to_XYZ(const float &src_x,
                                              const float &lim_x,
                                              const float &y) {
  color3<float, xyz_tag> result;
  result.channels[0] = partial_iprod(cie_2006_X(), src_x, lim_x, y);
  result.channels[1] = partial_iprod(cie_2006_Y(), src_x, lim_x, y);
  result.channels[2] = partial_iprod(cie_2006_Z(), src_x, lim_x, y);
  return result;
}

// TODO: This is crap.  Use the SRGB -> XYZ conversion instead.
inline dense_signal srgb_to_spectral(const color3<float, srgb_tag> &src) {
  const float &r = src[0];
  const float &g = src[1];
  const float &b = src[2];

  dense_signal result = smits_zero();

  if (r < g && r < b) {
    result += r * smits_white();
    if (g < b) {
      result += (g - r) * smits_cyan();
      result += (b - g) * smits_blue();
    } else {
      result += (b - r) * smits_cyan();
      result += (g - b) * smits_green();
    }
  } else if (g < r && g < b) {
    result += g * smits_white();
    if (r < b) {
      result += (r - g) * smits_magenta();
      result += (b - r) * smits_blue();
    } else {
      result += (b - g) * smits_magenta();
      result += (r - b) * smits_red();
    }
  } else {
    result += b * smits_white();
    if (r < g) {
      result += (r - b) * smits_yellow();
      result += (g - r) * smits_green();
    } else {
      result += (g - b) * smits_yellow();
      result += (r - g) * smits_red();
    }
  }

  return result;
}

}  // namespace ballistae

#endif
