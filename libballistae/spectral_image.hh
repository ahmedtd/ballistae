#pragma once

#include <array>
#include <cstddef>
#include <numeric>
#include <ostream>
#include <sstream>
#include <type_traits>
#include <vector>

#include "libballistae/span.hh"

namespace ballistae {

struct spectral_image {
  std::size_t row_size;
  std::size_t col_size;
  std::size_t freq_size;

  float freq_min;
  float freq_max;

  std::vector<float> power_density_sums;
  std::vector<float> power_density_counts;

 public:
  spectral_image();
  spectral_image(std::size_t row_size_in, std::size_t col_size_in,
                 std::size_t freq_size_in, float freq_min_in,
                 float freq_max_in);

  void resize(std::size_t row_size, std::size_t col_size,
              std::size_t freq_size);

  void record_sample(std::size_t r, std::size_t c, float freq,
                     float power_density);

  struct sample {
    span<float> freq_span;
    float power_density_sum;
    float power_density_count;
  };

  sample read_sample(std::size_t r, std::size_t c, std::size_t f);
};

enum class read_spectral_image_error {
  ok = 0,
  error_reading_header_length,
  error_reading_header,
  error_bad_data_layout_version,
  error_decompressing,
};

std::string read_spectral_image_error_to_string(read_spectral_image_error err);

read_spectral_image_error read_spectral_image(spectral_image *im,
                                              std::istream *in);

enum class write_spectral_image_error {
  ok = 0,
  error_writing_header_length,
  error_writing_header,
  error_compressing,
};

write_spectral_image_error write_spectral_image(spectral_image *im,
                                                std::ostream *out);

}  // namespace ballistae
