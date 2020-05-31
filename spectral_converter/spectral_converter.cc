#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "libballistae/color.hh"
#include "libballistae/spectral_image.hh"
#include "third_party/cc/absl/absl/flags/flag.h"
#include "third_party/cc/absl/absl/flags/parse.h"
#include "third_party/cc/absl/absl/strings/str_format.h"
#include "third_party/jpeg/turbojpeg.h"

ABSL_FLAG(std::string, input, "", "input spectral image");

ABSL_FLAG(std::string, output, "", "output jpeg file");
ABSL_FLAG(int, jpeg_quality, 75, "output jpeg quality");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  std::string input = absl::GetFlag(FLAGS_input);
  if (input == "") {
    std::cerr << "--input must be specified" << std::endl;
    return 1;
  }

  std::ifstream input_stream(input, std::ifstream::binary);
  if (!input_stream) {
    std::cerr << absl::StreamFormat("could not open input file %s\n", input);
    return 1;
  }

  std::string output = absl::GetFlag(FLAGS_output);
  if (output == "") {
    std::cerr << "--output must be specified" << std::endl;
    return 1;
  }

  ballistae::spectral_image image;
  auto read_err = ballistae::read_spectral_image(&image, &input_stream);
  if (read_err != ballistae::read_spectral_image_error::ok) {
    std::cerr << absl::StreamFormat(
        "Problem reading input file %s: %s\n", input,
        read_spectral_image_error_to_string(read_err));
    return 1;
  }

  std::vector<ballistae::color3<float, ballistae::xyz_tag>> xyz_image(
      image.row_size * image.col_size);
  for (std::size_t r = 0; r < image.row_size; r++) {
    for (std::size_t c = 0; c < image.col_size; c++) {
      ballistae::color3<float, ballistae::xyz_tag> xyz_sum{0, 0, 0};
      for (std::size_t f = 0; f < image.wavelength_size; f++) {
        auto sample = image.read_sample(r, c, f);

        float average_energy =
            sample.power_density_count == 0.0f
                ? 0.0f
                : sample.power_density_sum / sample.power_density_count;

        xyz_sum += ballistae::spectral_to_XYZ(sample.wavelength_span.lo,
                                              sample.wavelength_span.hi,
                                              average_energy);
      }
      xyz_image[r * image.col_size + c] = xyz_sum;
    }
  }

  // Find max luminance (Y channel) in image, and rescale so that it has value
  // 100.  Nonscientific, and will not allow dark or dim images, but should give
  // good results for now while I work on plumbing principled handling of power
  // density through the renderer.
  float max_luminance = -1.0;
  for (std::size_t r = 0; r < image.row_size; r++) {
    for (std::size_t c = 0; c < image.col_size; c++) {
      if (xyz_image[r * image.col_size + c].channels[1] > max_luminance) {
        max_luminance = xyz_image[r * image.col_size + c].channels[1];
      }
    }
  }
  if (max_luminance > 100.0f) {
    for (std::size_t r = 0; r < image.row_size; r++) {
      for (std::size_t c = 0; c < image.col_size; c++) {
        xyz_image[r * image.col_size + c].channels[0] *= 100.0f / max_luminance;
        xyz_image[r * image.col_size + c].channels[1] *= 100.0f / max_luminance;
        xyz_image[r * image.col_size + c].channels[2] *= 100.0f / max_luminance;
      }
    }
  }

  std::vector<unsigned char> srgb_image(image.row_size * image.col_size * 3);
  for (std::size_t r = 0; r < image.row_size; r++) {
    for (std::size_t c = 0; c < image.col_size; c++) {
      auto srgb = ballistae::clamp_01(
          ballistae::xyz_to_srgb(xyz_image[r * image.col_size + c]));
      std::size_t base_index = r * image.col_size * 3 + c * 3;
      srgb_image[base_index + 0] = (unsigned char)(srgb.channels[0] * 255.0f);
      srgb_image[base_index + 1] = (unsigned char)(srgb.channels[1] * 255.0f);
      srgb_image[base_index + 2] = (unsigned char)(srgb.channels[2] * 255.0f);
    }
  }

  long unsigned int jpeg_length = 0;
  unsigned char* jpeg_image = NULL;
  tjhandle compressor = tjInitCompress();

  int compress_err = tjCompress2(
      compressor, srgb_image.data(), image.row_size, 0, image.col_size,
      TJPF_RGB, &jpeg_image, &jpeg_length, TJSAMP_444,
      absl::GetFlag(FLAGS_jpeg_quality), TJFLAG_FASTDCT);
  if (compress_err != 0) {
    std::cerr << absl::StreamFormat("while compressing image: %s\n",
                                    tjGetErrorStr2(compressor));
    return 1;
  }

  std::ofstream output_stream(output, std::ofstream::binary);
  output_stream.write(reinterpret_cast<char*>(jpeg_image), jpeg_length);
  if (!output_stream.good()) {
    std::cerr << "error while writing output file" << std::endl;
    return 1;
  }

  tjDestroy(compressor);
  tjFree(jpeg_image);

  return 0;
}
