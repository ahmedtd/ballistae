#include <sstream>

#include "gtest/gtest.h"
#include "libballistae/spectral_image.hh"
#include "libballistae/spectral_image_file.pb.h"

TEST(SpectralImage, RecordSample) {
  ballistae::spectral_image im1(1, 1, 10, 100.0, 200.0);

  im1.record_sample(0, 0, 0, 1.0f);
  im1.record_sample(0, 0, 1, 1.0f);
  im1.record_sample(0, 0, 2, 1.0f);
  im1.record_sample(0, 0, 3, 1.0f);
  im1.record_sample(0, 0, 4, 1.0f);
  im1.record_sample(0, 0, 5, 1.0f);
  im1.record_sample(0, 0, 6, 1.0f);
  im1.record_sample(0, 0, 7, 1.0f);
  im1.record_sample(0, 0, 8, 1.0f);
  im1.record_sample(0, 0, 9, 1.0f);
}

TEST(SpectralImage, BasicRoundTrip) {
  std::stringstream memstream(std::stringstream::in | std::stringstream::out |
                              std::stringstream::binary);

  ballistae::spectral_image im1(1, 2, 3, 0.0f, 1.0f);
  im1.record_sample(0, 0, 0.0f, 3.14159f);
  im1.record_sample(0, 1, 0.9f, 1337.0f);

  auto write_err = ballistae::write_spectral_image(&im1, &memstream);
  ASSERT_EQ(write_err, ballistae::write_spectral_image_error::ok);

  ballistae::spectral_image im2;

  auto read_err = ballistae::read_spectral_image(&im2, &memstream);
  ASSERT_EQ(read_err, ballistae::read_spectral_image_error::ok);

  EXPECT_EQ(im1.row_size, im2.row_size);
  EXPECT_EQ(im1.col_size, im2.col_size);
  EXPECT_EQ(im1.wavelength_size, im2.wavelength_size);
  EXPECT_EQ(im1.wavelength_min, im2.wavelength_min);
  EXPECT_EQ(im1.wavelength_max, im2.wavelength_max);
  EXPECT_EQ(im1.power_density_sums, im2.power_density_sums);
  EXPECT_EQ(im1.power_density_counts, im2.power_density_counts);
}

TEST(SpectralImage, Cut) {
  ballistae::spectral_image big(10, 11, 12, 0.0f, 1.0f);
  big.record_sample(3, 5, 0.0f, 1.0f);

  ballistae::spectral_image small;
  big.cut(&small, 3, 5, 5, 9);

  EXPECT_EQ(small.row_size, 2);
  EXPECT_EQ(small.col_size, 4);
  EXPECT_EQ(small.wavelength_size, big.wavelength_size);
  EXPECT_EQ(small.wavelength_min, big.wavelength_min);
  EXPECT_EQ(small.wavelength_max, big.wavelength_max);

  ballistae::spectral_image::sample sample = small.read_sample(0, 0, 0);
  EXPECT_EQ(sample.power_density_sum, 1.0f);
  EXPECT_EQ(sample.power_density_count, 1.0f);
}

TEST(SpectralImage, Paste) {
  ballistae::spectral_image big(10, 11, 12, 0.0f, 1.0f);

  ballistae::spectral_image small(3, 4, 12, 0.0f, 1.0f);
  small.record_sample(1, 1, 1, 1.0f);

  big.paste(&small, 1, 2);

  ballistae::spectral_image::sample sample = big.read_sample(2, 3, 1);
  EXPECT_EQ(sample.power_density_sum, 1.0f);
  EXPECT_EQ(sample.power_density_count, 1.0f);
}
