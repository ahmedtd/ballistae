#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "libballistae/spectral_image.hh"
#include "libballistae/spectral_image_file.pb.h"
#include "libballistae/zipstream.hh"

namespace ballistae {

spectral_image::spectral_image()
    : row_size(0), col_size(0), freq_size(0), freq_min(0.0), freq_max(0.0) {}

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

void spectral_image::resize(std::size_t row_size, std::size_t col_size,
                            std::size_t freq_size) {
  this->row_size = row_size;
  this->col_size = col_size;
  this->freq_size = freq_size;

  this->power_density_sums.resize(row_size * col_size * freq_size);
  std::fill(this->power_density_sums.begin(), this->power_density_sums.end(),
            0.0);
  this->power_density_counts.resize(row_size * col_size * freq_size);
  std::fill(this->power_density_counts.begin(),
            this->power_density_counts.end(), 0.0);
}

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

  im->freq_min = hdr.frequency_min();
  im->freq_max = hdr.frequency_max();

  im->resize(hdr.row_size(), hdr.col_size(), hdr.frequency_size());

  ::ballistae::zipreader reader;
  if (reader.open(in, 1024 * 1024) != zipreader_error::ok) {
    return read_spectral_image_error::error_decompressing;
  }

  std::size_t n = sizeof(float) * im->power_density_sums.size();
  zipreader_error ret =
      reader.read(reinterpret_cast<char *>(im->power_density_sums.data()), n);
  if (reader.last_read_size != n || ret != zipreader_error::ok) {
    std::cerr << "didn't read enough data for sums" << std::endl;
    std::cerr << "read " << reader.last_read_size << std::endl;
    reader.close();
    return read_spectral_image_error::error_decompressing;
  }

  n = sizeof(float) * im->power_density_counts.size();
  ret =
      reader.read(reinterpret_cast<char *>(im->power_density_counts.data()), n);
  if (reader.last_read_size != n ||
      (ret != zipreader_error::ok && ret != zipreader_error::error_eof)) {
    std::cerr << "didn't read enough data for counts" << std::endl;
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
  hdr.set_frequency_size(im->freq_size);
  hdr.set_frequency_min(im->freq_min);
  hdr.set_frequency_max(im->freq_max);

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

  std::vector<char> zlib_output_buffer(1024 * 1024);

  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  if (deflateInit(&strm, 9)) {
    return write_spectral_image_error::error_compressing;
  }

  // Write power_density_sums
  do {
    strm.avail_out = zlib_output_buffer.size();
    strm.next_out =
        reinterpret_cast<unsigned char *>(zlib_output_buffer.data());

    strm.avail_in = sizeof(float) * im->power_density_sums.size();
    strm.next_in =
        reinterpret_cast<unsigned char *>(im->power_density_sums.data());

    int ret = deflate(&strm, Z_NO_FLUSH);
    if (ret == Z_STREAM_ERROR) {
      deflateEnd(&strm);
      return write_spectral_image_error::error_compressing;
    }

    std::size_t used_out = zlib_output_buffer.size() - strm.avail_out;
    out->write(zlib_output_buffer.data(), used_out);
    if (!out->good()) {
      deflateEnd(&strm);
      return write_spectral_image_error::error_compressing;
    }
  } while (strm.avail_out == 0);

  // Write power_density_counts
  do {
    strm.avail_out = zlib_output_buffer.size();
    strm.next_out =
        reinterpret_cast<unsigned char *>(zlib_output_buffer.data());

    strm.avail_in = sizeof(float) * im->power_density_counts.size();
    strm.next_in =
        reinterpret_cast<unsigned char *>(im->power_density_counts.data());

    int ret = deflate(&strm, Z_FINISH);
    if (ret == Z_STREAM_ERROR) {
      deflateEnd(&strm);
      return write_spectral_image_error::error_compressing;
    }

    std::size_t used_out = zlib_output_buffer.size() - strm.avail_out;
    out->write(zlib_output_buffer.data(), used_out);
    if (!out->good()) {
      deflateEnd(&strm);
      return write_spectral_image_error::error_compressing;
    }
  } while (strm.avail_out == 0);

  deflateEnd(&strm);

  return write_spectral_image_error::ok;
}

}  // namespace ballistae
