#include <libballistae/scene.hh>

#include <random>
#include <vector>

#include <armadillo>

#define cimg_display 0
#define cimg_verbosity 1
#include <CImg.h>

#include <libballistae/camera_plugin_interface.hh>
#include <libballistae/geom_plugin_interface.hh>
#include <libballistae/matr_plugin_interface.hh>
#include <libballistae/ray.hh>
#include <libballistae/spectrum.hh>

namespace ballistae
{

void update_merge_min(
    span<double> *min_spans_src,
    span<double> *min_spans_lim,
    arma::vec3   *min_normals_src,
    std::size_t  *min_geominds_src,
    span<double> *new_spans_src,
    arma::vec3   *new_normals_src,
    std::size_t new_geomind
)
{
    for(; min_spans_src != min_spans_lim; ++min_spans_src)
    {
        if(*new_spans_src < *min_spans_src)
        {
            *min_spans_src = *new_spans_src;
            min_normals_src[0] = new_normals_src[0];
            min_normals_src[1] = new_normals_src[1];
            *min_geominds_src = new_geomind;
        }

        min_normals_src += 2;
        ++min_geominds_src;

        ++new_spans_src;
        new_normals_src += 2;
    }
}

void ray_intersect_batch(
    const dray3 *query_src,
    const dray3 *query_lim,
    const span<double> &must_overlap,
    const scene &the_scene,
    span<double> *out_spans_src,
    arma::vec3 *out_normals_src,
    std::size_t *out_geominds_src
)
{
    span<double> *out_spans_lim = out_spans_src + (query_lim - query_src);

    std::size_t *out_geominds_lim = out_geominds_src + (query_lim - query_src);

    // Initialize the contact output buffer to all infinities.
    std::fill(
        out_spans_src,
        out_spans_lim,
        span<double>{span<double>::inf(), span<double>::inf()}
    );

    // Initialize the index output buffer to all sentinels.
    std::fill(out_geominds_src, out_geominds_lim, the_scene.geometries.size());

    // A buffer for the current geometry's contact output.  Note that there are
    // two normals for every span.
    std::vector<span<double>> cur_spans(query_lim - query_src);
    std::vector<arma::vec3>   cur_normals(2 * (query_lim - query_src));


    for(std::size_t cur_geomind = 0;
        cur_geomind < the_scene.geometries.size();
        ++cur_geomind)
    {

        the_scene.geometries[cur_geomind]->ray_intersect(
            query_src,
            query_lim,
            must_overlap,
            0,
            cur_spans.data(),
            cur_normals.data()
        );

        update_merge_min(
            out_spans_src,
            out_spans_lim,
            out_normals_src,
            out_geominds_src,
            cur_spans.data(),
            cur_normals.data(),
            cur_geomind
        );
    }
}

void shade_batch(
    const dray3 *query_src,
    const dray3 *query_lim,
    const span<double> *spans_src,
    const arma::vec3   *normals_src,
    const std::size_t  *geominds_src,
    const scene &the_scene,
    color_d_rgb *shades_out_src
)
{
    // For now, dispatch them serially.

    for(; query_src != query_lim;
        ++query_src, ++spans_src, normals_src += 2, ++geominds_src,
            ++shades_out_src)
    {
        if(*geominds_src < the_scene.materials.size())
        {
            *shades_out_src = the_scene.materials[*geominds_src]->shade(
                *query_src,
                *spans_src,
                normals_src
            );
        }
        else
        {
            *shades_out_src = {0, 0, 0};
        }
    }
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

cimg::CImg<float>& render_scene(
    cimg::CImg<float> &img,
    const std::shared_ptr<const camera_priv> &the_camera,
    const scene &the_scene,
    unsigned int ss_factor
)
{
    std::mt19937 ss_pert_engn(1235);
    std::uniform_real_distribution<double> ss_pert_dist(0.0, 1.0);

    std::size_t img_rows = static_cast<std::size_t>(img.width());
    std::size_t img_cols = static_cast<std::size_t>(img.height());

    std::size_t sample_rows = (img_rows << ss_factor);
    std::size_t sample_cols = (img_cols << ss_factor);

    std::size_t samples = sample_rows * sample_cols;

    std::vector<dray3> queries(samples);
    std::vector<span<double>> spans(samples);
    std::vector<arma::vec3> normals(2 * samples);
    std::vector<std::size_t> geominds(samples);
    std::vector<color_d_rgb> shades(samples);

    // Generate queries
    for(std::size_t r = 0; r < sample_rows; ++r)
    {
        for(std::size_t c = 0; c < sample_cols; ++c)
        {
            arma::vec3 image_coords = scan_plane_to_image_space(
                r, sample_rows,
                c, sample_cols,
                ss_pert_engn,
                ss_pert_dist
            );

            queries[r*sample_cols + c] = the_camera->image_to_ray(image_coords);
        }
    }

    // Batch intersect rays to get contact info.
    ray_intersect_batch(
        queries.data(),
        queries.data() + queries.size(),
        {0.0, std::numeric_limits<double>::infinity()},
        the_scene,
        spans.data(),
        normals.data(),
        geominds.data()
    );

    // Batch shade contacts
    shade_batch(
        queries.data(),
        queries.data() + queries.size(),
        spans.data(),
        normals.data(),
        geominds.data(),
        the_scene,
        shades.data()
    );

    // Reduce the shade buffer into our hdr image.
    for(std::size_t r = 0; r < sample_rows; ++r)
    {
        for(std::size_t c = 0; c < sample_rows; ++c)
        {
            for(std::size_t channel = 0; channel < 3; ++channel)
            {
                img(
                    c >> ss_factor,
                    r >> ss_factor,
                    0,
                    channel
                ) += shades[r*sample_cols + c].channels[channel];
            }
        }
    }

    return img;
}

}
