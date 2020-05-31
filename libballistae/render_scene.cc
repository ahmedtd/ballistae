#include <fstream>
#include <future>
#include <thread>

#include "libballistae/render_scene.hh"
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

struct chunk_worker {
  spectral_image sample_db;

  std::mt19937 rng;

  std::function<void(std::size_t)> progress_function;

  std::size_t maxdepth;
  std::size_t target_samples;

  std::size_t img_rows;
  std::size_t img_cols;

  std::size_t row_src;
  std::size_t row_lim;

  std::size_t col_src;
  std::size_t col_lim;

  camera const *the_camera;
  scene const *the_scene;

  void render();
};

void chunk_worker::render() {
  std::size_t samples_collected = 0;
  for (std::size_t cr = this->row_src; cr < this->row_lim; ++cr) {
    for (std::size_t cc = this->col_src; cc < this->col_lim; ++cc) {
      for (std::size_t cw = 0; cw < this->sample_db.wavelength_size; ++cw) {
        std::size_t r = cr - this->row_src;
        std::size_t c = cc - this->col_src;

        spectral_image::sample samp = this->sample_db.read_sample(r, c, cw);
        if (samp.power_density_count >= this->target_samples) {
          continue;
        }
        std::size_t samples_to_add =
            this->target_samples - samp.power_density_count;

        for (std::size_t cs = 0; cs < samples_to_add; ++cs) {
          double lambda_cur = this->sample_db.wavelength_bin(cw).lo;

          auto image_coords = scan_plane_to_image_space(
              cr, this->img_rows, cc, this->img_cols, this->rng);

          dray3 cur_query =
              this->the_camera->image_to_ray(image_coords, this->rng);

          double sampled_power =
              sample_ray(cur_query, *(this->the_scene), lambda_cur, this->rng,
                         this->maxdepth) /
              (this->sample_db.wavelength_size);

          this->sample_db.record_sample(r, c, cw, sampled_power);
          samples_collected++;
        }
      }
    }

    this->progress_function(samples_collected);
    samples_collected = 0;
  }
}

void render_scene(options const &the_options, spectral_image *sample_db,
                  camera const &the_camera, scene const &the_scene,
                  std::function<void(size_t, size_t)> progress_function) {
  using std::min;

  size_t cur_progress = 0;
  std::mutex progress_mutex;

  // Count the total number of samples recorded in sample_db.  When we resume a
  // render, we don't want to just repeat our same rng choices again!
  std::size_t existing_samples = 0;
  for (std::size_t i = 0; i < sample_db->power_density_counts.size(); i++) {
    existing_samples += std::size_t(sample_db->power_density_counts[i]);
  }

  // Count the number of samples we want to have at the end of the render, for
  // reporting progress.
  std::size_t want_samples =
      the_options.target_subsamples * sample_db->power_density_counts.size();
  std::size_t total_samples = 0;
  if (want_samples > existing_samples) {
    total_samples = want_samples - existing_samples;
  }

  auto processor_count = std::thread::hardware_concurrency();
  if (processor_count == 0) {
    processor_count = 1;
  }

  // We chunk work by rows.
  std::size_t work_unit = sample_db->row_size / processor_count;

  std::vector<std::future<std::unique_ptr<chunk_worker>>> tasks;
  for (std::size_t i = 0; i < processor_count; ++i) {
    auto worker = std::make_unique<chunk_worker>();
    // TODO: Think about how to make this more repeatable.
    worker->rng = std::mt19937(existing_samples);
    worker->progress_function = [&](std::size_t sub_progress) {
      std::scoped_lock lock{progress_mutex};
      cur_progress += sub_progress;
      progress_function(cur_progress, total_samples);
    };
    worker->maxdepth = the_options.maxdepth;
    worker->target_samples = the_options.target_subsamples;
    worker->img_rows = sample_db->row_size;
    worker->img_cols = sample_db->col_size;
    worker->row_src = i * work_unit;
    worker->row_lim = min((i + 1) * work_unit, sample_db->row_size);
    worker->col_src = 0;
    worker->col_lim = sample_db->col_size;
    worker->the_camera = &the_camera;
    worker->the_scene = &the_scene;
    sample_db->cut(&(worker->sample_db), worker->row_src, worker->row_lim, 0,
                   sample_db->col_size);

    auto fn = [](std::unique_ptr<chunk_worker> worker) {
      worker->render();
      return worker;
    };
    tasks.emplace_back(std::async(std::launch::async, fn, std::move(worker)));
  }

  for (auto &task : tasks) {
    auto worker = task.get();
    sample_db->paste(&(worker->sample_db), worker->row_src, 0);
  }
}

}  // namespace ballistae
