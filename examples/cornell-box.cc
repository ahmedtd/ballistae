#include <fstream>
#include <iostream>
#include <string>

#include "libballistae/camera/pinhole.hh"
#include "libballistae/color.hh"
#include "libballistae/dense_signal.hh"
#include "libballistae/geometry/box.hh"
#include "libballistae/geometry/infinity.hh"
#include "libballistae/geometry/plane.hh"
#include "libballistae/geometry/sphere.hh"
#include "libballistae/geometry/surface_mesh.hh"
#include "libballistae/material/emitter.hh"
#include "libballistae/material/gauss.hh"
#include "libballistae/material/mc_lambert.hh"
#include "libballistae/material/nc_smooth.hh"
#include "libballistae/material/pc_smooth.hh"
#include "libballistae/material_map.hh"
#include "libballistae/render_scene.hh"
#include "libballistae/scene.hh"
#include "libballistae/spectral_image.hh"
#include "third_party/cc/absl/absl/flags/flag.h"
#include "third_party/cc/absl/absl/flags/parse.h"
#include "third_party/cc/absl/absl/strings/str_format.h"

ABSL_FLAG(std::string, output_file, "output.spectral",
          "Output spectral sample db");
ABSL_FLAG(std::size_t, output_rows, 512, "Output image rows");
ABSL_FLAG(std::size_t, output_cols, 512, "Output image cols");
ABSL_FLAG(std::size_t, wavelength_bins, 25, "Output wavelength bins");
ABSL_FLAG(float, wavelength_min, 390.0, "Output wavelength min");
ABSL_FLAG(float, wavelength_max, 935.0, "Output wavelength max");

ABSL_FLAG(std::size_t, render_target_subsamples, 4,
          "Number of subsamples to collect from each pixel and frequency bin");
ABSL_FLAG(std::size_t, render_maxdepth, 8,
          "Maximum depth of bounces to consider");

ABSL_FLAG(bool, resume, false,
          "Should we re-open our output file, and add more samples");

using namespace frustum;
using namespace ballistae;

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);

  options the_options;
  the_options.maxdepth = absl::GetFlag(FLAGS_render_maxdepth);
  the_options.target_subsamples = absl::GetFlag(FLAGS_render_target_subsamples);

  std::string output_file = absl::GetFlag(FLAGS_output_file);

  spectral_image sample_db;
  if (absl::GetFlag(FLAGS_resume)) {
    std::ifstream in(output_file, std::ifstream::binary);
    if (!in) {
      std::cerr << absl::StreamFormat(
          "Resumption requested, but specified file %s doesn't exist\n",
          output_file);
      return 1;
    }

    auto read_err = read_spectral_image(&sample_db, &in);
    if (read_err != read_spectral_image_error::ok) {
      std::cerr << absl::StreamFormat(
          "Resumption requested, but encounted error reading %s: %s\n",
          output_file, read_spectral_image_error_to_string(read_err));
      return 1;
    }

    if (sample_db.row_size != absl::GetFlag(FLAGS_output_rows)) {
      std::cerr << absl::StreamFormat(
          "Resumption requested, but the existing spectral image doesn't have "
          "the right number of rows (got %d, want %d)\n",
          sample_db.row_size, absl::GetFlag(FLAGS_output_rows));
      return 1;
    }

    if (sample_db.col_size != absl::GetFlag(FLAGS_output_cols)) {
      std::cerr << absl::StreamFormat(
          "Resumption requested, but the existing spectral image doesn't have "
          "the right number of cols (got %d, want %d)\n",
          sample_db.col_size, absl::GetFlag(FLAGS_output_cols));
      return 1;
    }

    if (sample_db.wavelength_size != absl::GetFlag(FLAGS_wavelength_bins)) {
      std::cerr << absl::StreamFormat(
          "Resumption requested, but the existing spectral image doesn't have "
          "the right number of wavelength bins (got %d, want %d)\n",
          sample_db.wavelength_size, absl::GetFlag(FLAGS_wavelength_bins));
      return 1;
    }
  } else {
    std::ifstream in(output_file, std::ifstream::binary);
    if (in) {
      std::cerr << absl::StreamFormat(
          "Resumption not requested, but specified file %s exists\n",
          output_file);
      return 1;
    }

    sample_db = spectral_image(absl::GetFlag(FLAGS_output_rows),
                               absl::GetFlag(FLAGS_output_cols),
                               absl::GetFlag(FLAGS_wavelength_bins),
                               absl::GetFlag(FLAGS_wavelength_min),
                               absl::GetFlag(FLAGS_wavelength_max));
  }

  scene the_scene;

  auto cie_d65_emitter = materials::make_emitter(
      material_map::make_constant_spectrum(10 * cie_d65<double>()));
  auto cie_a_emitter = materials::make_emitter(
      material_map::make_constant_spectrum(10 * cie_a<double>()));
  auto matte = materials::make_gauss(material_map::make_constant_scalar(0.5));
  auto matte2 = materials::make_gauss(material_map::make_constant_scalar(0.05));
  auto green_matte = materials::make_mc_lambert(
      material_map::make_constant_spectrum(smits_green<double>()));
  auto glass =
      materials::make_nc_smooth(material_map::make_constant_spectrum(
                                    ramp<double>(390.0, 835.0, 89, 1.7, 1.5)),
                                material_map::make_constant_scalar(1.0));

  infinity infinity;
  sphere sphere;

  surface_mesh bunny = surface_mesh_from_obj_file("bunny.obj", true);

  box center_box({span<double>{0, 0.5}, {0, 0.5}, {0, 0.5}});

  box ground(
      {span<double>{0, 10.1}, span<double>{0, 10.1}, span<double>{-0.5, 0}});
  box roof(
      {span<double>{0, 10.1}, span<double>{0, 10.1}, span<double>{10, 10.1}});
  box wall_e(
      {span<double>{10, 10.1}, span<double>{0, 10}, span<double>{0, 10}});
  box wall_n(
      {span<double>{0, 10}, span<double>{10, 10.1}, span<double>{0, 10}});
  box wall_w({span<double>{-0.1, 0}, span<double>{0, 10}, span<double>{0, 10}});
  box wall_s({span<double>{0, 10}, span<double>{-0.1, 0}, span<double>{0, 10}});

  the_scene.elements = {
      {&infinity, &cie_d65_emitter, affine_transform<double, 3>::identity()},
      {&bunny, &cie_a_emitter,
       affine_transform<double, 3>::translation({5, 4, 0}) *
           affine_transform<double, 3>::scaling(10)},
      {&bunny, &matte,
       affine_transform<double, 3>::translation({4, 5, 0}) *
           affine_transform<double, 3>::scaling(10)},
      {&ground, &matte2, affine_transform<double, 3>::identity()},
      {&roof, &matte2, affine_transform<double, 3>::identity()},
      {&wall_n, &green_matte, affine_transform<double, 3>::identity()},
      {&wall_w, &green_matte, affine_transform<double, 3>::identity()},
      {&wall_s, &green_matte, affine_transform<double, 3>::identity()},
      {&center_box, &glass,
       affine_transform<double, 3>::translation({3, 3, 0})}};

  crush(the_scene, 0.0);

  pinhole the_camera({1, 1, 2}, {1, 0, 0, 0, 1, 0, 0, 0, 1},
                     {0.02, 0.018, 0.012});

  the_camera.set_eye(fixvec<double, 3>{5, 5, 1} - the_camera.center);

  render_scene(the_options, &sample_db, the_camera, the_scene,
               [](size_t cur, size_t tot) {
                 std::cerr << absl::StreamFormat("\r%d/%d %d%%", cur, tot,
                                                 (cur / (tot / 100)));
               });

  std::cerr << "\n";

  std::ofstream out(output_file, std::ofstream::binary);
  auto write_err = write_spectral_image(&sample_db, &out);
  if (write_err != write_spectral_image_error::ok) {
    std::cerr << absl::StreamFormat("Error writing output file\n");
    return 1;
  }

  return 0;
}
