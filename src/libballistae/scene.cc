#include <libballistae/scene.hh>
#include <libballistae/render_scene.hh>

#include <atomic>
#include <random>
#include <vector>

#include <armadillo>

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
        const auto &geom  = the_scene.elements[i].the_geometry;
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

static inline arma::vec3 scan_plane_to_image_space(
    std::size_t cur_row,
    std::size_t img_rows,
    std::size_t cur_col,
    std::size_t img_cols,
    std::ranlux24 ss_pert_engn,
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
    size_t sample_index,
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
            lambda_nm,
            sample_index,
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

struct sample_scratch
{
    sample_scratch(size_t s)
        : c_stack(s),
          r_stack(s),
          p_stack(s),
          k_stack(s)
    {
    }

    std::vector<size_t>         c_stack;
    std::vector<ray<double, 3>> r_stack;
    std::vector<double>         p_stack;
    std::vector<double>         k_stack;

    size_t sample_idx;
};

double sample_ray(
    const dray3 &initial_query,
    const scene &the_scene,
    double lambda_nm,
    std::ranlux24 &thread_rng,
    const std::vector<size_t> &l_stack,
    sample_scratch &scratch
)
{
    using std::pow;

    size_t depth_lim = l_stack.size();

    // Load the initial query into cell 0.  The final result will appear in
    // p_stack[0].
    scratch.p_stack[0] = 0.0;
    scratch.k_stack[0] = 1.0;
    scratch.r_stack[0] = initial_query;

    std::fill(scratch.c_stack.begin(), scratch.c_stack.end(), 0);

    size_t i = 1;
    do
    {
        if(i == depth_lim)
        {
            // We bottom out at fixed depth.

            shade_info<double> shading = shade_ray(
                the_scene,
                scratch.r_stack[i-1],
                lambda_nm,
                scratch.sample_idx,
                thread_rng
            );

            scratch.p_stack[i-1]
                += scratch.k_stack[i-1] * shading.emitted_power / l_stack[i-1];
            --i;
        }
        else if(scratch.c_stack[i] < l_stack[i])
        {
            shade_info<double> shading = shade_ray(
                the_scene,
                scratch.r_stack[i-1],
                lambda_nm,
                scratch.sample_idx,
                thread_rng
            );

            if(scratch.c_stack[i] == 0)
                scratch.p_stack[i] = 0.0;

            scratch.p_stack[i] += shading.emitted_power;
            scratch.k_stack[i] = shading.propagation_k;
            scratch.r_stack[i] = shading.incident_ray;

            ++scratch.c_stack[i];

            // Only move down a level if we will actually use any of the power.
            // Also, any material that returns a 0.0 for propagation_k probably
            // did not provide a real ray to recurse on.
            if(scratch.k_stack[i] != 0.0)
            {
                ++i;
            }
        }
        else
        {
            scratch.p_stack[i-1]
                += (scratch.k_stack[i-1] * scratch.p_stack[i]) / l_stack[i-1];
            scratch.c_stack[i] = 0;
            --i;
        }
        ++scratch.sample_idx;
    }
    while(i != 0);

    return scratch.p_stack[0];
}

color_d_XYZ shade_pixel(
    const std::size_t cur_row,
    const std::size_t cur_col,
    const std::size_t img_rows,
    const std::size_t img_cols,
    const camera &the_camera,
    const scene &the_scene,
    unsigned int ss_factor,
    std::ranlux24 &rng,
    const std::vector<size_t> &sampling_profile,
    const std::vector<double> &lambda_nm_profile,
    sample_scratch &scratch
)
{
    std::uniform_real_distribution<double> ss_pert_dist(0.0, 1.0);

    color_d_XYZ result = {{{0, 0, 0}}};

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

            dray3 cur_query = the_camera.image_to_ray(image_coords);

            for(double lambda_nm : lambda_nm_profile)
            {
                double sampled_power = sample_ray(
                    cur_query,
                    the_scene,
                    lambda_nm,
                    rng,
                    sampling_profile,
                    scratch
                );

                result += spectral_to_XYZ(lambda_nm, sampled_power);
            }
        }
    }

    return result;
}


image<float> render_scene(
    const std::size_t img_rows,
    const std::size_t img_cols,
    const camera &the_camera,
    const scene &the_scene,
    unsigned int ss_factor,
    std::atomic_size_t &cur_progress,
    const std::vector<size_t> &sampling_profile,
    const std::vector<double> &lambda_nm_profile
)
{
    std::ranlux24 thread_rng(1235);
    sample_scratch scratch(sampling_profile.size());

    image<float> result({img_rows, img_cols, 3}, 0.0f);

# pragma omp parallel for              \
    firstprivate(thread_rng, scratch) \
    schedule(dynamic, 16)
    for(size_t cr = 0; cr < img_rows; ++cr)
    {
        scratch.sample_idx = thread_rng();
        for(size_t cc = 0; cc < img_cols; ++cc)
        {
            color_d_XYZ cur_XYZ = shade_pixel(
                cr, cc,
                img_rows, img_cols,
                the_camera,
                the_scene,
                ss_factor,
                thread_rng,
                sampling_profile,
                lambda_nm_profile,
                scratch
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
