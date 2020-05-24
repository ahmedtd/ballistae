#include <cmath>

#include "libballistae/color.hh"

namespace ballistae {

static float srgb_gamma_correct(float raw) {
  using std::pow;
  if (raw < 0.0031308f) {
    return 12.92f * raw;
  }
  return 1.055f * pow(raw, 1.0f / 2.4f) - 0.055f;
}

color3<float, srgb_tag> xyz_to_srgb(const color3<float, xyz_tag> &xyz) {
  // First normalize the XYZ input so that the Y channel of a D65 input is 1.
  // D65 in XYZ is (95.047, 100.0, 108.883).
  float s_x = xyz.channels[0] / 100.0f;
  float s_y = xyz.channels[1] / 100.0f;
  float s_z = xyz.channels[2] / 100.0f;

  // Apply the linear transform from the sRGB spec.
  float r_r = 3.240479f * s_x - 1.537150f * s_y + -0.498535 * s_z;
  float r_g = -0.969265f * s_x + 1.875992f * s_y + 0.041556f * s_z;
  float r_b = 0.055648f * s_x - 0.204043f * s_y + 1.057311f * s_z;

  // Apply gamma correction from the sRGB spec.
  float r = srgb_gamma_correct(r_r);
  float g = srgb_gamma_correct(r_g);
  float b = srgb_gamma_correct(r_b);

  return color3<float, srgb_tag>{r, g, b};
}

static float srgb_gamma_uncorrect(float corrected) {
  using std::pow;
  if (corrected < 0.04045f) {
    return corrected / 12.92f;
  }
  return pow((corrected + 0.055f) / 1.055f, 2.4f);
}

color3<float, xyz_tag> srgb_to_xyz(const color3<float, srgb_tag> &srgb) {
  // Apply inverse gamma correction.
  float r_r = srgb_gamma_uncorrect(srgb.channels[0]);
  float r_g = srgb_gamma_uncorrect(srgb.channels[1]);
  float r_b = srgb_gamma_uncorrect(srgb.channels[2]);

  // Apply linear transformation from the sRGB spec.
  float s_x = 0.4123908f * r_r + 0.35758434f * r_g + 0.18048079f * r_b;
  float s_y = 0.21263901f * r_r + 0.71516868f * r_g + 0.07219232f * r_b;
  float s_z = 0.01933082f * r_r + 0.11919478f * r_g + 0.95053215f * r_b;

  // These numbers are scaled so that D65 Y is 1, so scale them back to its full
  // value.
  float x = s_x * 100.0f;
  float y = s_y * 100.0f;
  float z = s_z * 100.0f;

  return color3<float, xyz_tag>{x, y, z};
}

}  // namespace ballistae
