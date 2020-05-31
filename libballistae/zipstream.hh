#pragma once

#include <istream>
#include <ostream>
#include <vector>

#include "third_party/zlib/zlib.h"

namespace ballistae {

enum class zipreader_error {
  ok = 0,
  error_decompressing,
  error_early_eof,
  error_eof,
};

struct zipreader {
  z_stream stream;
  std::istream *in;
  std::vector<char> buffer;
  std::size_t last_read_size;

  zipreader_error open(std::istream *in_in, std::size_t bufsize);

  void close();

  zipreader_error read(char *buf, std::size_t n);
};

enum class zipwriter_error {
  ok = 0,
  error_compressing,
  error_output_full,
};

struct zipwriter {
  z_stream stream;
  std::ostream *out;
  std::vector<char> buffer;
  std::size_t last_write_size;

  zipwriter_error open(std::ostream *out, std::size_t bufsize,
                       int compress_level);

  zipwriter_error close();

  zipwriter_error write(char *buf, std::size_t n);
};

}  // namespace ballistae
