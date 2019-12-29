#include <sstream>

#include "gtest/gtest.h"
#include "libballistae/spectral_image.hh"
#include "libballistae/spectral_image_file.pb.h"

TEST(SpectralImage, BasicRoundTrip) {
  std::stringstream memstream(std::stringstream::in | std::stringstream::out |
                              std::stringstream::binary);

  ballistae::spectral_image im1(10, 11, 12, 0.0, 1.0);
  im1.record_sample(0, 0, 0.0, 3.14159);
  im1.record_sample(9, 10, 0.9, 1337.0);

  auto write_err = ballistae::write_spectral_image(&im1, &memstream);
  ASSERT_EQ(write_err, ballistae::write_spectral_image_error::ok);

  ballistae::spectral_image im2;

  auto read_err = ballistae::read_spectral_image(&im2, &memstream);
  ASSERT_EQ(read_err, ballistae::read_spectral_image_error::ok);

  EXPECT_EQ(im1.row_size, im2.row_size);
  EXPECT_EQ(im1.col_size, im2.col_size);
  EXPECT_EQ(im1.freq_size, im2.freq_size);
  EXPECT_EQ(im1.freq_min, im2.freq_min);
  EXPECT_EQ(im1.freq_max, im2.freq_max);
  EXPECT_EQ(im1.power_density_sums, im2.power_density_sums);
  EXPECT_EQ(im1.power_density_counts, im2.power_density_counts);
}
