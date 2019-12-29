#include "libballistae/render_scene.hh"

#include <fstream>

#include "libballistae/spectral_image.hh"

namespace ballistae {

fixvec<double, 3> scan_plane_to_image_space(std::size_t cur_row,
                                            std::size_t img_rows,
                                            std::size_t cur_col,
                                            std::size_t img_cols,
                                            std::mt19937 &thread_rng) {
  double d_cur_col = static_cast<double>(cur_col);
  double d_cur_row = static_cast<double>(cur_row);
  double d_img_cols = static_cast<double>(img_cols);
  double d_img_rows = static_cast<double>(img_rows);

  std::uniform_real_distribution<double> ss_dist(0.0, 1.0);

  double y = 1.0 - 2.0 * (d_cur_col - ss_dist(thread_rng)) / d_img_cols;
  double z = 1.0 - 2.0 * (d_cur_row - ss_dist(thread_rng)) / d_img_rows;

  return {1.0, y, z};
}

shade_info<double> shade_ray(const scene &the_scene, const dray3 &reflected_ray,
                             double lambda_cur) {
  ray_segment<double, 3> refl_query = {
      reflected_ray,
      {epsilon<double>(), std::numeric_limits<double>::infinity()}};

  contact<double> glb_contact;
  const crushed_scene_element *hit_element;
  std::tie(glb_contact, hit_element) =
      scene_ray_intersect(the_scene, refl_query);

  if (hit_element != nullptr) {
    auto shade_result =
        hit_element->the_material->shade(glb_contact, lambda_cur);

    return shade_result;
  } else {
    return shade_info<double>{0.0, 0, {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}};
  }
}

double sample_ray(const dray3 &initial_query, const scene &the_scene,
                  double lambda_cur, std::mt19937 &thread_rng,
                  size_t depth_lim) {
  double accum_power = 0.0;
  double cur_k = 1.0;
  ray<double, 3> cur_ray = initial_query;

  for (size_t i = 0; i < depth_lim && cur_k != 0.0; ++i) {
    shade_info<double> shading = shade_ray(the_scene, cur_ray, lambda_cur);

    accum_power += cur_k * shading.emitted_power;
    cur_k = shading.propagation_k;
    cur_ray = shading.incident_ray;
  }

  return accum_power;
}

void render_scene(const options &the_options, const camera &the_camera,
                  const scene &the_scene,
                  std::function<void(size_t, size_t)> progress_function) {
  spectral_image power_density_samples(
      the_options.img_rows, the_options.img_cols,
      the_options.gridsize * the_options.gridsize, the_options.lambda_min,
      the_options.lambda_max);

  size_t cur_progress = 0;

#pragma omp parallel
  {
    std::mt19937 thread_rng;

#pragma omp for schedule(dynamic, 128)
    for (size_t cr = 0; cr < the_options.img_rows; ++cr) {
      for (size_t cc = 0; cc < the_options.img_cols; ++cc) {
        for (size_t i = 0; i < the_options.gridsize * the_options.gridsize;
             ++i) {
          double lambda_cur =
              the_options.lambda_min +
              double(i) * (the_options.lambda_max - the_options.lambda_min) /
                  double(the_options.gridsize * the_options.gridsize);

          auto image_coords = scan_plane_to_image_space(
              cr, the_options.img_rows, cc, the_options.img_cols, thread_rng);

          dray3 cur_query = the_camera.image_to_ray(image_coords, thread_rng);

          double sampled_power = sample_ray(cur_query, the_scene, lambda_cur,
                                            thread_rng, the_options.maxdepth) /
                                 (the_options.gridsize * the_options.gridsize);

          power_density_samples.record_sample(cr, cc, lambda_cur,
                                              sampled_power);

          // cur_color +=
          //     spectral_to_XYZ(lambda_cur - sample_bandwidth / 2,
          //                     lambda_cur + sample_bandwidth / 2,
          //                     sampled_power);
        }
        // color_d_rgb cur_rgb = clamp_0(to_rgb(result));

        // for (size_t chan = 0; chan < 3; ++chan) {
        //   result(cr, cc, chan) = cur_rgb.channels[chan];
        // }
      }

#pragma omp atomic
      cur_progress += the_options.img_cols;
#pragma omp flush(cur_progress)

#pragma omp critical
      progress_function(cur_progress,
                        the_options.img_rows * the_options.img_cols);
    }
  }

  std::fstream output(the_options.output_file, std::fstream::out |
                                                   std::fstream::trunc |
                                                   std::fstream::binary);
  write_spectral_image(&power_density_samples, &output);
}

}  // namespace ballistae
