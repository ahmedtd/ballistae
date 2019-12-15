#ifndef LIBBALLISTAE_MATERIAL_MAP_HH
#define LIBBALLISTAE_MATERIAL_MAP_HH

#include <cstddef>

#include "include/frustum-0/indicial/fixed.hh"
#include "include/libballistae/dense_signal.hh"
#include "include/libballistae/vector.hh"

namespace ballistae {

struct material_coords {
  fixvec<double, 2> mtl2;
  fixvec<double, 3> mtl3;
  double lambda;
};

namespace material_map {

inline auto make_constant_scalar(double scalar) {
  return [=](const material_coords &map) { return scalar; };
}

inline auto make_constant_spectrum(dense_signal<double> spectrum) {
  return [=](const material_coords &map) {
    return interpolate(spectrum, map.lambda);
  };
}

template <class InputT, class InputA, class InputB>
auto make_lerp(InputT t, InputA a, InputB b) {
  return [=](const material_coords &map) {
    double tval = t(map);
    return (1 - tval) * a(map) + tval * b(map);
  };
}

template <class InputT, class InputA, class InputB>
auto make_level(double t_switch, InputT t, InputA a, InputB b) {
  return [=](const material_coords &map) {
    double tval = t(map);

    if (tval < t_switch)
      return a(map);
    else
      return b(map);
  };
}

template <class InputA>
auto make_clamp(double min, double max, InputA a) {
  return [=](const material_coords &map) {
    double a_val = a(map);
    if (a_val < min) return min;
    if (a_val >= max) return max;
    return a_val;
  };
}

inline auto make_checkerboard_surface(double period) {
  return [=](const material_coords &map) {
    using std::floor;

    unsigned char parity = 0;
    for (size_t i = 0; i < 2; ++i) {
      double q = map.mtl2(i) / period;
      double f = floor(q);

      double rem = q - f;

      unsigned char cur = (rem > 0.5) ? 1 : 0;
      parity ^= cur;
    }

    return parity & 0x1u;
  };
}

inline auto make_checkerboard_volume(double period) {
  return [=](const material_coords &map) {
    using std::floor;

    unsigned char parity = 0;
    for (size_t i = 0; i < 3; ++i) {
      double q = map.mtl3(i) / period;
      double f = floor(q);

      double rem = q - f;

      unsigned char cur = (rem > 0.5) ? 1 : 0;
      parity ^= cur;
    }

    return (double(parity & 0x1u));
  };
}

inline auto make_bullseye_surface(double period) {
  return [=](const material_coords &map) {
    using std::fmod;

    double d = norm(map.mtl2) / period;

    return (fmod(d, 1.0) < 0.5) ? 0 : 1;
  };
}

inline auto make_bullseye_volume(double period) {
  return [=](const material_coords &map) {
    using std::fmod;

    double d = norm(map.mtl3) / period;

    return (fmod(d, 1.0) < 0.5) ? 0 : 1;
  };
}

// A multiplicative hash (in Knuth's style), that makes use of the fact that we
// only use 24 input bits.
//
// The multiplicative constant is floor(2^24 / (golden ratio)), tweaked a bit to
// avoid attractors above 0xc in the last digit.
inline uint32_t hashmul(uint32_t x) {
  x = ((x >> 16) ^ x) * 0x45d9f3b;
  x = ((x >> 16) ^ x) * 0x45d9f3b;
  x = ((x >> 16) ^ x);
  return x;
}

template <class Field>
Field perlin_dotgrad(fixvec<size_t, 3> c, fixvec<Field, 3> d) {
  // Map the low four bits hash, and map them to one of the 12 unit integer
  // gradients.  Then, take the dot product of d with the selected gradient.
  // This is an improvement from Perlin's 2002 paper.

  uint32_t hash =
      ((c(0) & 0xffu) << 16) | ((c(1) & 0xffu) << 8) | ((c(2) & 0xffu) << 0);

  hash = hashmul(hash);

  switch (hash & 0xfu) {
    case 0x0u:
      return d(0) + d(1);
    case 0x1u:
      return d(0) - d(1);
    case 0x2u:
      return -d(0) + d(1);
    case 0x3u:
      return -d(0) - d(1);

    case 0x4u:
      return d(1) + d(2);
    case 0x5u:
      return d(1) - d(2);
    case 0x6u:
      return -d(1) + d(2);
    case 0x7u:
      return -d(1) - d(2);

    case 0x8u:
      return d(2) + d(0);
    case 0x9u:
      return d(2) - d(0);
    case 0xau:
      return -d(2) + d(0);
    case 0xbu:
      return -d(2) - d(0);

    case 0xcu:
      return d(0) + d(1);
    case 0xdu:
      return -d(0) + d(1);
    case 0xeu:
      return -d(1) + d(2);
    case 0xfu:
      return -d(1) - d(2);
  };

  return 0;  // Dead code.
}

template <class Field>
Field fade(Field x) {
  return x * x * x * (x * (x * 6 - 15) + 10);
}

template <class Field>
Field lerp(Field t, Field a, Field b) {
  return (1 - t) * a + t * b;
}

inline auto make_perlin_surface(double period) {
  return [=](const material_coords &map) {
    fixvec<double, 3> x_abs = {map.mtl2(0) * 256.0 / period,
                               map.mtl2(1) * 256.0 / period, 0.0};

    using std::floor;
    using std::lrint;

    using iv = fixvec<size_t, 3>;
    using fv = fixvec<double, 3>;

    fixvec<long, 3> cell_long = lrint(floor(x_abs));
    fixvec<size_t, 3> cell = {static_cast<size_t>(cell_long(0) & 0xff),
                              static_cast<size_t>(cell_long(1) & 0xff),
                              static_cast<size_t>(cell_long(2) & 0xff)};
    fixvec<double, 3> x_rel = x_abs - floor(x_abs);

    return lerp(fade(x_rel(1)),
                lerp(fade(x_rel(0)),
                     perlin_dotgrad(cell + iv{0, 0, 0}, x_rel - fv{0, 0, 0}),
                     perlin_dotgrad(cell + iv{1, 0, 0}, x_rel - fv{1, 0, 0})),
                lerp(fade(x_rel(0)),
                     perlin_dotgrad(cell + iv{0, 1, 0}, x_rel - fv{0, 1, 0}),
                     perlin_dotgrad(cell + iv{1, 1, 0}, x_rel - fv{1, 1, 0})));
  };
}

inline auto make_perlin_volume(double period) {
  return [=](const material_coords &map) {
    fixvec<double, 3> x_abs = map.mtl3 * 256.0 / period;

    using std::floor;
    using std::lrint;

    using iv = fixvec<size_t, 3>;
    using fv = fixvec<double, 3>;

    fixvec<long, 3> cell_long = lrint(floor(x_abs));
    fixvec<size_t, 3> cell = {static_cast<size_t>(cell_long(0) & 0xff),
                              static_cast<size_t>(cell_long(1) & 0xff),
                              static_cast<size_t>(cell_long(2) & 0xff)};
    fixvec<double, 3> x_rel = x_abs - floor(x_abs);

    return lerp(
        fade(x_rel(2)),
        lerp(fade(x_rel(1)),
             lerp(fade(x_rel(0)),
                  perlin_dotgrad(cell + iv{0, 0, 0}, x_rel - fv{0, 0, 0}),
                  perlin_dotgrad(cell + iv{1, 0, 0}, x_rel - fv{1, 0, 0})),
             lerp(fade(x_rel(0)),
                  perlin_dotgrad(cell + iv{0, 1, 0}, x_rel - fv{0, 1, 0}),
                  perlin_dotgrad(cell + iv{1, 1, 0}, x_rel - fv{1, 1, 0}))),
        lerp(fade(x_rel(1)),
             lerp(fade(x_rel(0)),
                  perlin_dotgrad(cell + iv{0, 0, 1}, x_rel - fv{0, 0, 1}),
                  perlin_dotgrad(cell + iv{1, 0, 1}, x_rel - fv{1, 0, 1})),
             lerp(fade(x_rel(0)),
                  perlin_dotgrad(cell + iv{0, 1, 1}, x_rel - fv{0, 1, 1}),
                  perlin_dotgrad(cell + iv{1, 1, 1}, x_rel - fv{1, 1, 1}))));
  };
}

}  // namespace material_map

}  // namespace ballistae

#endif
