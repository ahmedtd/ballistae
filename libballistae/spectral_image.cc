#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "libballistae/spectral_image.hh"
#include "libballistae/spectral_image_file.pb.h"
#include "libballistae/zipstream.hh"

namespace ballistae {

spectral_image::spectral_image()
    : row_size(0),
      col_size(0),
      wavelength_size(0),
      wavelength_min(0.0),
      wavelength_max(0.0) {}

spectral_image::spectral_image(std::size_t row_size_in, std::size_t col_size_in,
                               std::size_t wavelength_size_in,
                               float wavelength_min_in, float wavelength_max_in)
    : row_size(row_size_in),
      col_size(col_size_in),
      wavelength_size(wavelength_size_in),
      wavelength_min(wavelength_min_in),
      wavelength_max(wavelength_max_in),
      power_density_sums(row_size * col_size * wavelength_size, 0.0),
      power_density_counts(row_size * col_size * wavelength_size, 0.0) {}

void spectral_image::resize(std::size_t row_size, std::size_t col_size,
                            std::size_t wavelength_size) {
  this->row_size = row_size;
  this->col_size = col_size;
  this->wavelength_size = wavelength_size;

  this->power_density_sums.resize(row_size * col_size * wavelength_size);
  std::fill(this->power_density_sums.begin(), this->power_density_sums.end(),
            0.0);
  this->power_density_counts.resize(row_size * col_size * wavelength_size);
  std::fill(this->power_density_counts.begin(),
            this->power_density_counts.end(), 0.0);
}

span<float> spectral_image::wavelength_bin(std::size_t i) const {
  float bin_width = (this->wavelength_max - this->wavelength_min) /
                    float(this->wavelength_size);
  float lo = this->wavelength_min + float(i) * bin_width;

  if (i == this->wavelength_size - 1) {
    return span<float>{lo, this->wavelength_max};
  }

  float hi = this->wavelength_min + float(i + 1) * bin_width;
  return span<float>{lo, hi};
}

void spectral_image::record_sample(std::size_t row, std::size_t col,
                                   std::size_t wavelength_index,
                                   float power_density) {
  std::size_t sample_index = row * this->col_size * this->wavelength_size +
                             col * this->wavelength_size + wavelength_index;
  this->power_density_sums[sample_index] += power_density;
  this->power_density_counts[sample_index] += 1.0;
}

spectral_image::sample spectral_image::read_sample(std::size_t r, std::size_t c,
                                                   std::size_t f) const {
  std::size_t sample_index = r * this->col_size * this->wavelength_size +
                             c * this->wavelength_size + f;

  float wavelength_step =
      (this->wavelength_max - this->wavelength_min) / this->wavelength_size;

  spectral_image::sample sample;

  sample.wavelength_span =
      span<float>{f * wavelength_step + wavelength_min,
                  (f + 1) * wavelength_step + wavelength_min};
  sample.power_density_sum = this->power_density_sums[sample_index];
  sample.power_density_count = this->power_density_counts[sample_index];

  return sample;
}

void spectral_image::cut(spectral_image *dst, std::size_t row_src,
                         std::size_t row_lim, std::size_t col_src,
                         std::size_t col_lim) const {
  dst->resize(row_lim - row_src, col_lim - col_src, this->wavelength_size);
  dst->wavelength_min = this->wavelength_min;
  dst->wavelength_max = this->wavelength_max;

  for (std::size_t r = row_src; r < row_lim; r++) {
    for (std::size_t c = col_src; c < col_lim; c++) {
      for (std::size_t w = 0; w < this->wavelength_size; w++) {
        std::size_t src_index = r * this->col_size * this->wavelength_size +
                                c * this->wavelength_size + w;
        std::size_t dst_index =
            (r - row_src) * dst->col_size * dst->wavelength_size +
            (c - col_src) * dst->wavelength_size + w;

        dst->power_density_sums[dst_index] =
            this->power_density_sums[src_index];
        dst->power_density_counts[dst_index] =
            this->power_density_counts[src_index];
      }
    }
  }
}

void spectral_image::paste(spectral_image const *src, std::size_t row_src,
                           std::size_t col_src) {
  std::size_t row_lim = row_src + src->row_size;
  std::size_t col_lim = col_src + src->col_size;

  for (std::size_t r = row_src; r < row_lim; r++) {
    for (std::size_t c = col_src; c < col_lim; c++) {
      for (std::size_t w = 0; w < this->wavelength_size; w++) {
        std::size_t dst_index = r * this->col_size * this->wavelength_size +
                                c * this->wavelength_size + w;
        std::size_t src_index =
            (r - row_src) * src->col_size * src->wavelength_size +
            (c - col_src) * src->wavelength_size + w;

        this->power_density_sums[dst_index] =
            src->power_density_sums[src_index];
        this->power_density_counts[dst_index] =
            src->power_density_counts[src_index];
      }
    }
  }
}

std::string read_spectral_image_error_to_string(read_spectral_image_error err) {
  switch (err) {
    case read_spectral_image_error::ok:
      return "ok";
    case read_spectral_image_error::error_reading_header_length:
      return "error while reading header length";
    case read_spectral_image_error::error_reading_header:
      return "error while reading header";
    case read_spectral_image_error::error_bad_data_layout_version:
      return "got bad data layout version";
    case read_spectral_image_error::error_decompressing:
      return "error while decompressing";
    default:
      return "unknown error";
  }
}

read_spectral_image_error read_spectral_image(spectral_image *im,
                                              std::istream *in) {
  ::ballistae::spectral_image_file::SpectralImageHeader hdr;

  // Read header length
  std::size_t header_length;
  in->read(reinterpret_cast<char *>(&header_length), sizeof(std::size_t));
  if (!*in) {
    return read_spectral_image_error::error_reading_header_length;
  }

  // Read header
  std::vector<char> header_buf(header_length);
  in->read(header_buf.data(), header_length);
  if (!*in) {
    return read_spectral_image_error::error_reading_header;
  }
  if (!hdr.ParseFromArray(reinterpret_cast<void *>(header_buf.data()),
                          header_length)) {
    return read_spectral_image_error::error_reading_header;
  }

  if (hdr.data_layout_version() != 1) {
    return read_spectral_image_error::error_bad_data_layout_version;
  }

  im->wavelength_min = hdr.wavelength_min();
  im->wavelength_max = hdr.wavelength_max();

  im->resize(hdr.row_size(), hdr.col_size(), hdr.wavelength_size());

  ::ballistae::zipreader reader;
  if (reader.open(in, 1024 * 1024) != zipreader_error::ok) {
    return read_spectral_image_error::error_decompressing;
  }

  std::size_t n = sizeof(float) * im->power_density_sums.size();
  zipreader_error ret =
      reader.read(reinterpret_cast<char *>(im->power_density_sums.data()), n);
  if (reader.last_read_size != n || ret != zipreader_error::ok) {
    reader.close();
    return read_spectral_image_error::error_decompressing;
  }

  n = sizeof(float) * im->power_density_counts.size();
  ret =
      reader.read(reinterpret_cast<char *>(im->power_density_counts.data()), n);
  if (reader.last_read_size != n ||
      (ret != zipreader_error::ok && ret != zipreader_error::error_eof)) {
    reader.close();
    return read_spectral_image_error::error_decompressing;
  }

  reader.close();

  return read_spectral_image_error::ok;
}

write_spectral_image_error write_spectral_image(spectral_image *im,
                                                std::ostream *out) {
  ::ballistae::spectral_image_file::SpectralImageHeader hdr;
  hdr.set_row_size(im->row_size);
  hdr.set_col_size(im->col_size);
  hdr.set_wavelength_size(im->wavelength_size);
  hdr.set_wavelength_min(im->wavelength_min);
  hdr.set_wavelength_max(im->wavelength_max);

  hdr.set_data_layout_version(1);

  std::uint64_t header_size = hdr.ByteSizeLong();
  out->write(reinterpret_cast<char *>(&header_size), sizeof(std::uint64_t));
  if (!*out) {
    return write_spectral_image_error::error_writing_header_length;
  }

  std::vector<char> header_buf(header_size);
  if (!hdr.SerializeToArray(reinterpret_cast<void *>(header_buf.data()),
                            header_size)) {
    return write_spectral_image_error::error_writing_header;
  }
  out->write(header_buf.data(), header_size);
  if (!*out) {
    return write_spectral_image_error::error_writing_header;
  }

  ::ballistae::zipwriter writer;
  auto writer_err = writer.open(out, 1024 * 1024);
  if (writer_err != ::ballistae::zipwriter_error::ok) {
    return write_spectral_image_error::error_compressing;
  }

  writer_err =
      writer.write(reinterpret_cast<char *>(im->power_density_sums.data()),
                   sizeof(float) * im->power_density_sums.size());
  if (writer_err != ::ballistae::zipwriter_error::ok) {
    return write_spectral_image_error::error_compressing;
  }

  writer_err =
      writer.write(reinterpret_cast<char *>(im->power_density_counts.data()),
                   sizeof(float) * im->power_density_counts.size());
  if (writer_err != ::ballistae::zipwriter_error::ok) {
    return write_spectral_image_error::error_compressing;
  }

  writer_err = writer.close();
  if (writer_err != ::ballistae::zipwriter_error::ok) {
    return write_spectral_image_error::error_compressing;
  }

  return write_spectral_image_error::ok;
}

}  // namespace ballistae
