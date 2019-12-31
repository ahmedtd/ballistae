#include <algorithm>
#include <sstream>
#include <vector>

#include "gtest/gtest.h"
#include "libballistae/zipstream.hh"
#include "rapidcheck/gtest.h"

TEST(ZipStream, BasicRoundTrip) {
  std::stringstream memstream(std::stringstream::in | std::stringstream::out |
                              std::stringstream::binary);

  std::vector<char> data = {0x41, 0x41, 0x41, 0x41};

  ::ballistae::zipwriter writer;
  auto write_err = writer.open(&memstream, 8);
  ASSERT_EQ(write_err, ::ballistae::zipwriter_error::ok);

  write_err = writer.write(data.data(), data.size());
  ASSERT_EQ(write_err, ::ballistae::zipwriter_error::ok);

  write_err = writer.close();
  ASSERT_EQ(write_err, ::ballistae::zipwriter_error::ok);

  ::ballistae::zipreader reader;
  auto read_err = reader.open(&memstream, 8);
  ASSERT_EQ(read_err, ::ballistae::zipreader_error::ok);

  std::vector<char> recovered_data(4);
  read_err = reader.read(recovered_data.data(), recovered_data.size());
  ASSERT_TRUE(read_err == ::ballistae::zipreader_error::ok ||
              read_err == ::ballistae::zipreader_error::error_eof);
  ASSERT_EQ(reader.last_read_size, 4);

  // We have now read all the data, so a subsequent read should return EOF.
  std::vector<char> recovered_data2(4);
  read_err = reader.read(recovered_data2.data(), recovered_data2.size());
  ASSERT_EQ(read_err, ::ballistae::zipreader_error::error_eof);
  ASSERT_EQ(reader.last_read_size, 0);

  reader.close();

  ASSERT_EQ(data, recovered_data);
}

RC_GTEST_PROP(ZipStream, EverythingRoundTrips, ()) {
  auto buf_size = *rc::gen::inRange(1, 1024 * 1024);
  auto data = *rc::gen::arbitrary<std::vector<char>>();

  std::stringstream memstream(std::stringstream::in | std::stringstream::out |
                              std::stringstream::binary);

  ::ballistae::zipwriter writer;
  auto write_err = writer.open(&memstream, buf_size);
  RC_ASSERT(write_err == ::ballistae::zipwriter_error::ok);

  write_err = writer.write(data.data(), data.size());
  RC_ASSERT(write_err == ::ballistae::zipwriter_error::ok);

  write_err = writer.close();
  RC_ASSERT(write_err == ::ballistae::zipwriter_error::ok);

  ::ballistae::zipreader reader;
  auto read_err = reader.open(&memstream, buf_size);
  RC_ASSERT(read_err == ::ballistae::zipreader_error::ok);

  using std::max;

  std::vector<char> recovered_data(max(data.size(), std::size_t(1)));
  read_err = reader.read(recovered_data.data(), recovered_data.size());
  RC_ASSERT(read_err == ::ballistae::zipreader_error::ok ||
            read_err == ::ballistae::zipreader_error::error_eof);
  RC_ASSERT(reader.last_read_size == data.size());
  auto num_chars_read = reader.last_read_size;

  // We have now read all the data, so a subsequent read should return EOF.
  std::vector<char> recovered_data2(max(data.size(), std::size_t(1)));
  read_err = reader.read(recovered_data2.data(), recovered_data2.size());
  RC_ASSERT(read_err == ::ballistae::zipreader_error::error_eof);
  RC_ASSERT(reader.last_read_size == 0);

  reader.close();

  recovered_data.resize(num_chars_read);
  RC_ASSERT(data == recovered_data);
}
