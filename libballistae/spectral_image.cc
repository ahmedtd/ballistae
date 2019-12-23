#include "libballistae/spectral_image.hh"

#include "libballistae/spectral_image_file.pb.h"
#include "third_party/zlib/zlib.h"

namespace ballistae {

spectral_image::spectral_image(std::size_t row_size_in, std::size_t col_size_in,
                               std::size_t freq_size_in, float freq_min_in,
                               float freq_max_in)
    : row_size(row_size_in),
      col_size(col_size_in),
      freq_size(freq_size_in),
      freq_min(freq_min_in),
      freq_max(freq_max_in),
      power_density_sums(row_size * col_size * freq_size, 0.0),
      power_density_counts(row_size * col_size * freq_size, 0.0) {}

void spectral_image::record_sample(std::size_t row, std::size_t col, float freq,
                                   float power_density) {
  std::size_t freq_index =
      std::size_t((freq - this->freq_min) / (this->freq_max - this->freq_min) *
                  this->freq_size);
  std::size_t sample_index = row * this->col_size * this->freq_size +
                             col * this->freq_size + freq_index;
  this->power_density_sums[sample_index] += power_density;
  this->power_density_counts[sample_index] += 1.0;
}

enum class write_spectral_image_error {
  ok = 0,
  err,
};

void write_spectral_image(spectral_image *im, std::ostream *out) {
  ::ballistae::spectral_image_file::SpectralImageHeader hdr;
  hdr.mutable_row_size() = std::uint32(im->row_size);
  hdr.mutable_col_size() = std::uint32(im->col_size);
  hdr.mutable_frequency_size() = std::uint32(im->freq_size);
  hdr.mutable_frequency_min() = im->freq_min;
  hdr.mutable_frequency_max() = im->freq_max;

  if (!hdr.SerializeToOstream(out)) {
    std::cerr << "Failed to write image header" << std::endl;
    return;
  }

  std::vector<char> zlib_output_buffer(1024 * 1024);

  z_strm strm;
  strm.z_alloc = Z_NULL;
  strm.z_free = Z_NULL;
  strm.opaque = Z_NULL;
  if (deflateInit(&strm, 9)) {
    std::cerr << "Failed to initialize zlib output stream" << std::endl;
    return;
  }

  // Write power_density_sums
  do {
    strm.avail_out = zlib_output_buffer.size();
    strm.next_out = zlib_output_buffer
                        .data()

                            strm.avail_in =
        sizeof(float) * im->power_density_sums.size();
    strm.next_in = reinterpret_cast<char *>(im->power_density_sums.data());

    int ret = deflate(&strm, Z_FINISH);
    if (ret == Z_STRM_ERROR) {
      std::cerr << "Failed to deflate power density sums" << std::endl;
      deflateEnd(&strm);
      return;
    }

    std::size_t used_out = zlib_output_buffer.size() - strm.avail_out;
    output.write(zlib_output_buffer.data(), used_out);
    if (output.fail()) {
      std::cerr << "Failed to write deflated power density sums to output"
                << std::endl;
      deflateEnd(&strm);
      return;
    }
  } while (strm.avail_out == 0);

  // Write power_density_counts
  do {
    strm.avail_out = zlib_output_buffer.size();
    strm.next_out = reinterpret_cast<char *>(zlib_output_buffer.data());

    strm.avail_in = sizeof(float) * im->power_density_counts.size();
    strm.next_in = reinterpret_cast<char *>(im->power_density_counts.data());

    int ret = deflate(&strm, Z_FINISH);
    if (ret == Z_STRM_ERROR) {
      std::cerr << "Failed to deflate power density counts" << std::endl;
      deflateEnd(&strm);
      return;
    }

    std::size_t used_out = zlib_output_buffer.size() - strm.avail_out;
    output.write(zlib_output_buffer.data(), used_out);
    if (output.fail()) {
      std::cerr << "Failed to write deflated power density counts to output"
                << std::endl;
      deflateEnd(&strm);
      return;
    }
  } while (strm.avail_out == 0);

  deflateEnd(&strm);
}
