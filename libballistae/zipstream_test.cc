#include <sstream>
#include <vector>

#include "gtest/gtest.h"
#include "libballistae/zipstream.hh"

TEST(ZipStream, BasicRoundTrip) {
  std::stringstream memstream(std::stringstream::in | std::stringstream::out |
                              std::stringstream::binary);

  std::vector<char> data = {0x41, 0x41, 0x41, 0x41};

  ::ballistae::zipwriter writer;
  auto write_err = writer.open(&memstream, 32);
  ASSERT_EQ(write_err, ::ballistae::zipwriter_error::ok);

  write_err = writer.write(data.data(), data.size());
  ASSERT_EQ(write_err, ::ballistae::zipwriter_error::ok);

  write_err = writer.close();
  ASSERT_EQ(write_err, ::ballistae::zipwriter_error::ok);

  ::ballistae::zipreader reader;
  auto read_err = reader.open(&memstream, 14);
  ASSERT_EQ(read_err, ::ballistae::zipreader_error::ok);

  std::vector<char> recovered_data(4);
  read_err = reader.read(recovered_data.data(), recovered_data.size());
  ASSERT_EQ(read_err, ::ballistae::zipreader_error::ok);
  ASSERT_EQ(reader.last_read_size, 4);

  // We have now read all the data, so a subsequent read should return EOF.
  std::vector<char> recovered_data2(4);
  read_err = reader.read(recovered_data2.data(), recovered_data2.size());
  ASSERT_EQ(read_err, ::ballistae::zipreader_error::error_eof);
  ASSERT_EQ(reader.last_read_size, 0);

  reader.close();

  ASSERT_EQ(data, recovered_data);
}
