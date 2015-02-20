#include <libballistae/scene.hh>

#include <random>

#include <armadillo>

#define cimg_display 0
#define cimg_verbosity 1
#include <CImg.h>

#include <libballistae/camera_plugin_interface.hh>
#include <libballistae/contact.hh>
#include <libballistae/geom_plugin_interface.hh>
#include <libballistae/matr_plugin_interface.hh>
#include <libballistae/ray.hh>
#include <libballistae/spectrum.hh>

namespace ballistae
{

color_d_rgb ray_intersect(const dray3 &query, const scene &the_scene)
{
    contact least_contact = contact::infinity();
    std::size_t least_matr = the_scene.geometries.size();

    for(std::size_t i = 0; i < the_scene.geometries.size(); ++i)
    {
        auto cur_contact = the_scene.geometries[i]->ray_intersect(query);

        // Automagically handles NaN's.
        if(cur_contact < least_contact && cur_contact.t > 0.0)
        {
            least_contact = cur_contact;
            least_matr = i;
        }
    }

    if(least_matr == the_scene.geometries.size())
    {
        return the_scene.infty_matr->shade(query, least_contact);
    }
    else
    {
        return the_scene.materials[least_matr]->shade(query, least_contact);
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

    for(std::size_t cur_row = 0; cur_row < (img_rows << ss_factor); ++cur_row)
    {
        for(std::size_t cur_col = 0; cur_col < (img_cols << ss_factor); ++cur_col)
        {
            arma::vec3 image_coords = scan_plane_to_image_space(
                cur_row, (img_rows << ss_factor),
                cur_col, (img_cols << ss_factor),
                ss_pert_engn,
                ss_pert_dist
            );

            dray3 query = the_camera->image_to_ray(image_coords);

            color_d_rgb cur_color = ray_intersect(query, the_scene);

            for(std::size_t channel = 0; channel < 3; ++channel)
            {
                img(
                    cur_col >> ss_factor,
                    cur_row >> ss_factor,
                    0,
                    channel
                ) += cur_color.channels[channel];
            }
        }
    }

    return img;
}

}
