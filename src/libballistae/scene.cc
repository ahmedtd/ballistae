#include <libballistae/scene.hh>
#include <libballistae/render_scene.hh>

#include <atomic>
#include <random>
#include <vector>

#include <libballistae/camera.hh>
#include <libballistae/color.hh>
#include <libballistae/contact.hh>
#include <libballistae/geometry.hh>
#include <libballistae/illuminator.hh>
#include <libballistae/image.hh>
#include <libballistae/material.hh>
#include <libballistae/ray.hh>
#include <libballistae/span.hh>

namespace ballistae
{

std::tuple<contact<double>, size_t> scene_ray_intersect(
    const scene &the_scene,
    const ray_segment<double, 3> &query,
    std::ranlux24 &thread_rng
)
{
    contact<double> min_contact;
    min_contact.t = std::numeric_limits<double>::infinity();
    size_t min_ind = the_scene.elements.size();

    for(size_t i = 0; i < the_scene.elements.size(); ++i)
    {
        const auto geom  = the_scene.elements[i].the_geometry;
        const auto &ftran = the_scene.elements[i].forward_transform;
        const auto &rtran = the_scene.elements[i].reverse_transform;

        auto mdl_query = ftran * query;

        contact<double> mdl_contact = geom->ray_into(
            the_scene,
            mdl_query,
            thread_rng
        );

        contact<double> glb_contact = rtran * mdl_contact;

        if(glb_contact.t <= min_contact.t)
        {
            min_contact = glb_contact;
            min_ind = i;
        }

        // Now check if there is a closer exit.

        mdl_contact = geom->ray_exit(
            the_scene,
            mdl_query,
            thread_rng
        );

        glb_contact = rtran * mdl_contact;

        if(glb_contact.t <= min_contact.t)
        {
            min_contact = glb_contact;
            min_ind = i;
        }
    }

    return std::make_tuple(min_contact, min_ind);
}

static fixvec<double, 3> scan_plane_to_image_space(
    std::size_t cur_row,
    std::size_t img_rows,
    std::size_t cur_col,
    std::size_t img_cols,
    std::ranlux24 &ss_pert_engn,
    std::uniform_real_distribution<double> &ss_pert_dist
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
    double lambda_src,
    double lambda_lim,
    double lambda_cur,
    std::ranlux24 &thread_rng
)
{
    ray_segment<double, 3> refl_query = {
        reflected_ray,
        {epsilon<double>(), std::numeric_limits<double>::infinity()}
    };

    contact<double> glb_contact;
    size_t element_index;
    std::tie(glb_contact, element_index) = scene_ray_intersect(
        the_scene,
        refl_query,
        thread_rng
    );

    if(element_index < the_scene.elements.size())
    {
        const auto &matr = the_scene.elements[element_index].the_material;

        auto shade_result = matr->shade(
            the_scene,
            glb_contact,
            lambda_src,
            lambda_lim,
            lambda_cur,
            0,
            thread_rng
        );

        return shade_result;
    }
    else
    {
        return shade_info<double> {
            0.0,
            0,
            {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}
        };
    }
}

double sample_ray(
    const dray3 &initial_query,
    const scene &the_scene,
    double lambda_src,
    double lambda_lim,
    double lambda_cur,
    std::ranlux24 &thread_rng,
    size_t depth_lim
)
{
    double accum_power = 0.0;
    double cur_k = 1.0;
    ray<double, 3> cur_ray = initial_query;

    for(size_t i = 0; i < depth_lim && cur_k != 0.0; ++i)
    {
        shade_info<double> shading = shade_ray(
            the_scene,
            cur_ray,
            lambda_src,
            lambda_lim,
            lambda_cur,
            thread_rng
        );

        accum_power += cur_k * shading.emitted_power;
        cur_k = shading.propagation_k;
        cur_ray = shading.incident_ray;
    }

    return accum_power;
}

color_d_XYZ shade_pixel(
    const std::size_t cur_row,
    const std::size_t cur_col,
    const std::size_t img_rows,
    const std::size_t img_cols,
    const camera &the_camera,
    const scene &the_scene,
    unsigned int ss_gridsize,
    std::ranlux24 &thread_rng,
    const std::vector<double> &lambdas,
    const double sample_bandwidth,
    size_t depth_lim
)
{
    std::uniform_real_distribution<double> ss_dist(0.0, 1.0);
    std::uniform_real_distribution<double> lambda_dist(0.0, sample_bandwidth);

    color_d_XYZ result = {0, 0, 0};

    for(size_t sr = 0; sr < ss_gridsize; ++sr)
    {
        for(size_t sc = 0; sc < ss_gridsize; ++sc)
        {
            size_t idx = sr * ss_gridsize + sc;

            double lambda_src = lambdas[idx % lambdas.size()];
            double lambda_lim = lambda_src + sample_bandwidth;
            double lambda_cur = lambda_src + lambda_dist(thread_rng);

            auto image_coords = scan_plane_to_image_space(
                cur_row * ss_gridsize + sr, img_rows * ss_gridsize,
                cur_col * ss_gridsize + sc, img_cols * ss_gridsize,
                thread_rng,
                ss_dist
            );

            dray3 cur_query = the_camera.image_to_ray(image_coords, thread_rng);

            double sampled_power = sample_ray(
                cur_query,
                the_scene,
                lambda_src,
                lambda_lim,
                lambda_cur,
                thread_rng,
                depth_lim
            ) / (ss_gridsize * ss_gridsize);

            result += spectral_to_XYZ(lambda_src, lambda_lim, sampled_power);
        }
    }

    return result;
}

image<float> render_scene(
    const std::size_t img_rows,
    const std::size_t img_cols,
    const camera &the_camera,
    const scene &the_scene,
    const render_opts &opts,
    std::atomic_size_t &cur_progress
)
{
    std::ranlux24 thread_rng(1235);
    image<float> result({img_rows, img_cols, 3}, 0.0f);

    double lambda_step = measure(opts.bandwidth) / opts.n_lambdas;
    std::vector<double> lambdas(opts.n_lambdas);
    for(size_t i = 0; i < lambdas.size(); ++i)
        lambdas[i] = opts.bandwidth.lo + i * lambda_step;

# pragma omp parallel for firstprivate(thread_rng, lambdas) schedule(dynamic, 16)
    for(size_t cr = 0; cr < img_rows; ++cr)
    {
        for(size_t cc = 0; cc < img_cols; ++cc)
        {
            using std::shuffle;
            shuffle(lambdas.begin(), lambdas.end(), thread_rng);

            color_d_XYZ cur_XYZ = shade_pixel(
                cr, cc,
                img_rows, img_cols,
                the_camera,
                the_scene,
                opts.gridsize,
                thread_rng,
                lambdas,
                lambda_step,
                opts.depth_lim
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
