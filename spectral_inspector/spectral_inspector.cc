#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "libballistae/color.hh"
#include "libballistae/spectral_image.hh"
#include "third_party/cc/absl/absl/flags/flag.h"
#include "third_party/cc/absl/absl/flags/parse.h"
#include "third_party/cc/absl/absl/strings/str_format.h"

ABSL_FLAG(std::string, input, "", "input spectral image");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  std::string input = absl::GetFlag(FLAGS_input);
  if (input == "") {
    std::cerr << "--input must be specified" << std::endl;
    return 1;
  }

  std::ifstream input_stream(input, std::ifstream::binary);
  if (!input_stream) {
    std::cerr << "could not open input file " << input << std::endl;
    return 1;
  }

  ballistae::spectral_image image;
  auto read_err = ballistae::read_spectral_image(&image, &input_stream);
  if (read_err != ballistae::read_spectral_image_error::ok) {
    std::cerr << "problem reading input file " << input << ": "
              << read_spectral_image_error_to_string(read_err);
    return 1;
  }

  for (std::size_t r = 0; r < image.row_size; r++) {
    for (std::size_t c = 0; c < image.col_size; c++) {
      for (std::size_t f = 0; f < image.wavelength_size; f++) {
        auto sample = image.read_sample(r, c, f);

        if ((r * image.row_size + c) % 987 == 0) {
          std::cout << absl::StreamFormat("r=%d c=%d w=%d sum=%f count=%f\n", r,
                                          c, f, sample.power_density_sum,
                                          sample.power_density_count);
        }
      }
    }
  }
}
