#include <libballistae/scene.hh>
#include <libballistae/render_scene.hh>

#include <atomic>
#include <random>
#include <vector>

#include <armadillo>

#include <libballistae/camera_plugin_interface.hh>
#include <libballistae/geom_plugin_interface.hh>
#include <libballistae/image.hh>
#include <libballistae/matr_plugin_interface.hh>
#include <libballistae/ray.hh>
#include <libballistae/spectrum.hh>

namespace ballistae
{

std::tuple<span<double>, size_t> scene_ray_intersect(
    const scene &the_scene,
    const dray3 &query,
    const span<double> &must_overlap,
    std::mt19937 &thread_rng
)
{
    span<double> min_span = span<double>::inf();
    size_t min_geomind = the_scene.geometries.size();

    for(std::size_t cur_geomind = 0;
        cur_geomind < the_scene.geometries.size();
        ++cur_geomind)
    {
        span<double> cur_span
            = the_scene.geometries[cur_geomind]->ray_intersect(
                the_scene,
                query,
                must_overlap,
                thread_rng
            );

        if(cur_span < min_span)
        {
            min_span = cur_span;
            min_geomind = cur_geomind;
        }
    }

    return std::make_tuple(min_span, min_geomind);
}

static inline arma::vec3 scan_plane_to_image_space(
    std::size_t cur_row,
    std::size_t img_rows,
    std::size_t cur_col,
    std::size_t img_cols,
    std::mt19937 ss_pert_engn,
    std::uniform_real_distribution<double> ss_pert_dist
) {
    double d_cur_col = static_cast<double>(cur_col);
    double d_cur_row = static_cast<double>(cur_row);
    double d_img_cols = static_cast<double>(img_cols);
    double d_img_rows = static_cast<double>(img_rows);

    double y_pert = ss_pert_dist(ss_pert_engn);
    double z_pert = ss_pert_dist(ss_pert_engn);

    double y = 1.0 - 2.0 * (d_cur_col - y_pert) / d_img_cols;
    double z = 1.0 - 2.0 * (d_cur_row - z_pert) / d_img_rows;

    return {1.0, y, z};
}

shade_info<double> shade_ray(
    const scene &the_scene,
    const dray3 &reflected_ray,
    double lambda_nm,
    std::mt19937 &thread_rng
)
{
    span<double> c_span;
    size_t c_geomind;
    std::tie(c_span, c_geomind) = scene_ray_intersect(
        the_scene,
        reflected_ray,
        span<double>::pos_half(),
        thread_rng
    );

    if(c_geomind < the_scene.materials.size())
    {
        return the_scene.materials[c_geomind]->shade(
            the_scene,
            reflected_ray,
            c_span,
            lambda_nm,
            thread_rng
        );
    }
    else
    {
        return shade_info<double> {
            0.0,
            0.0,
            {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}
        };
    }
}

double sample_ray(
    const dray3 &initial_query,
    const scene &the_scene,
    double lambda_nm,
    std::mt19937 &thread_rng
)
{
    using std::pow;

    constexpr size_t depth_lim = 8;

    std::array<ray<double, 3>, depth_lim> r_stack;
    std::array<double, depth_lim>         p_stack;
    std::array<double, depth_lim>         k_stack;

    std::array<size_t, depth_lim> c_stack;
    c_stack.fill(0);

    std::array<size_t, depth_lim> l_stack = {0, 16, 16, 8, 8, 4, 4, 2};

    // Load the initial query into cell 0.  The final result will appear in
    // p_stack[0].
    p_stack[0] = 0.0;
    k_stack[0] = 1.0;
    r_stack[0] = initial_query;

    size_t i = 1;
    do
    {
        if(i == depth_lim)
        {
            // We bottom out at fixed depth.

            shade_info<double> shading = shade_ray(
                the_scene,
                r_stack[i-1],
                lambda_nm,
                thread_rng
            );

            p_stack[i-1] += k_stack[i-1] * shading.emitted_power;
            --i;
            continue;
        }
        else if(c_stack[i] < l_stack[i])
        {
            shade_info<double> shading = shade_ray(
                the_scene,
                r_stack[i-1],
                lambda_nm,
                thread_rng
            );

            if(c_stack[i] == 0)
                p_stack[i] = 0.0;

            p_stack[i] += shading.emitted_power;
            k_stack[i] = shading.propagation_k;
            r_stack[i] = shading.incident_ray;

            // Terminate this branch early if we won't use any power from lower
            // levels.
            if(k_stack[i] == 0.0)
            {
                ++c_stack[i];
                continue;
            }

            ++c_stack[i];
            ++i;
            continue;
        }
        else
        {
            p_stack[i-1] += k_stack[i-1] * p_stack[i];
            c_stack[i] = 0;
            --i;
            continue;
        }
    }
    while(i != 0);

    return p_stack[0];
}

color_d_XYZ shade_pixel(
    const std::size_t cur_row,
    const std::size_t cur_col,
    const std::size_t img_rows,
    const std::size_t img_cols,
    const std::shared_ptr<const camera_priv> &the_camera,
    const scene &the_scene,
    unsigned int ss_factor,
    std::mt19937 &rng
)
{
    std::uniform_real_distribution<double> ss_pert_dist(0.0, 1.0);

    size_t lambda_bins = 16;
    double lambda_src = 390;
    double lambda_lim = 835;
    double lambda_step = (lambda_lim - lambda_src) / (double) lambda_bins;
    std::uniform_real_distribution<double> lambda_dist(0, lambda_step);

    color_d_XYZ result = {0, 0, 0};

    for(size_t sr = 0; sr < (1u << ss_factor); ++sr)
    {
        for(size_t sc = 0; sc < (1u << ss_factor); ++sc)
        {
            arma::vec3 image_coords = scan_plane_to_image_space(
                (cur_row << ss_factor) | sr, img_rows << ss_factor,
                (cur_col << ss_factor) | sc, img_cols << ss_factor,
                rng,
                ss_pert_dist
            );

            dray3 cur_query = the_camera->image_to_ray(image_coords);

            for(size_t lambda_i = 0; lambda_i < 16; ++lambda_i)
            {
                double cur_lambda_nm = lambda_src
                    + lambda_i * lambda_step + lambda_dist(rng);

                double sampled_power = sample_ray(
                    cur_query,
                    the_scene,
                    cur_lambda_nm,
                    rng
                );

                result += spectral_to_XYZ(cur_lambda_nm, sampled_power);
            }
        }
    }

    return result;
}


image<float> render_scene(
    const std::size_t img_rows,
    const std::size_t img_cols,
    const std::shared_ptr<const camera_priv> &the_camera,
    const scene &the_scene,
    unsigned int ss_factor,
    std::atomic_size_t &cur_progress
)
{
    std::mt19937 thread_rng(1235);

    image<float> result({img_rows, img_cols, 3}, 0.0f);

# pragma omp parallel for firstprivate(thread_rng)
    for(size_t cr = 0; cr < img_rows; ++cr)
    {
        for(size_t cc = 0; cc < img_cols; ++cc)
        {
            color_d_XYZ cur_XYZ = shade_pixel(
                cr, cc,
                img_rows, img_cols,
                the_camera,
                the_scene,
                ss_factor,
                thread_rng
            );

            color_d_rgb cur_rgb = clamp_0(to_rgb(cur_XYZ));

            for(size_t chan = 0; chan < 3; ++chan)
            {
                result(cr, cc, chan) = cur_rgb.channels[chan];
            }
        }

        // atomic
        ++cur_progress;
    }

    return result;
}

}
