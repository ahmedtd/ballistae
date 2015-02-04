#include <cmath>

#include <random>
#include <tuple>
#include <vector>

#define cimg_display 0
#define cimg_verbosity 1
#include <CImg.h>

#include <ballistae/camera.hh>
#include <ballistae/contact.hh>
#include <ballistae/ray.hh>

#include <ballistae/cylinder.hh>
#include <ballistae/plane.hh>
#include <ballistae/sphere.hh>

#ifdef BALLISTAE_DEBUG
#include <iostream>
#endif

namespace cimg = cimg_library;

inline arma::vec3 scan_plane_to_image_space(
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

typedef std::array<float, 3> color_3f;

int main(int argc, char **argv)
{
    ballistae::camera camera(
        {0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0},
        {0.0, 0.0, 1.0},
        {2.0, 1.0, 1.0}
    );

    std::vector<color_3f> scene_materials = {
        {0.0f, 0.0f, 0.0f},
        {10.0f, 0.0f, 0.0f},
        {0.0f, 10.0f, 0.0f},
        {0.0f, 0.0f, 10.0f},
        {10.0f, 10.0f, 0.0f},
        {10.0f, 0.0f, 10.0f},
        {0.0f, 10.0f, 10.0f},
        {10.0f, 10.0f, 10.0f}
    };

    typedef std::tuple<ballistae::sphere, std::size_t> sphere_and_material;
    typedef std::tuple<ballistae::plane, std::size_t> plane_and_material;
    typedef std::tuple<ballistae::cylinder, std::size_t> cylinder_and_material;

    std::vector<sphere_and_material> scene_spheres = {
        std::make_tuple(ballistae::sphere({5.0,  1.0,  1.0}, 1.0), 1),
        std::make_tuple(ballistae::sphere({5.0,  0.2, -1.0}, 0.8), 2),
        std::make_tuple(ballistae::sphere({5.0, -0.2, -1.0}, 0.8), 4),
    };

    std::vector<plane_and_material> scene_planes = {
        std::make_tuple(ballistae::plane({5.0, 0.0, -10.0}, {0.0, 0.0, 1.0}), 5)
    };

    std::vector<cylinder_and_material> scene_cylinders = {
        std::make_tuple(ballistae::cylinder({20.0, 5.0, -10.0}, {5, -1, 1}, 4), 3)
    };

    std::size_t supersample_factor = 3;

    std::mt19937 ss_pert_engn(12345);
    std::uniform_real_distribution<double> ss_pert_dist(0.0, 1.0);

    std::size_t img_rows = 122;
    std::size_t img_cols = 180;

    cimg::CImg<float> hdr_buf(img_cols, img_rows, 1, 3);

    for(std::size_t cur_row = 0; cur_row < (img_rows << supersample_factor); ++cur_row)
    {
        for(std::size_t cur_col = 0; cur_col < (img_cols << supersample_factor); ++cur_col)
        {
            // Coordinates in image space
            arma::vec3 image_coords = scan_plane_to_image_space(
                cur_row, (img_rows << supersample_factor),
                cur_col, (img_cols << supersample_factor),
                ss_pert_engn,
                ss_pert_dist
            );

            ballistae::dray3 query = ballistae::camera_ray(
                camera,
                image_coords
            );

            std::size_t least_material = 0;
            ballistae::contact least_contact = ballistae::contact::infinity();

            for(const auto &geom_and_material : scene_spheres)
            {
                ballistae::contact cur_contact = ray_intersect(
                    query,
                    std::get<0>(geom_and_material)
                );

                if(cur_contact < least_contact && cur_contact.t > 1.0)
                {
                    least_contact = cur_contact;
                    least_material = std::get<1>(geom_and_material);
                }
            }

            for(const auto &geom_and_material : scene_planes)
            {
                ballistae::contact cur_contact = ray_intersect(
                    query,
                    std::get<0>(geom_and_material)
                );

                if(cur_contact < least_contact && cur_contact.t > 1.0)
                {
                    least_contact = cur_contact;
                    least_material = std::get<1>(geom_and_material);
                }
            }

            for(const auto &geom_and_material : scene_cylinders)
            {
                ballistae::contact cur_contact = ray_intersect(
                    query,
                    std::get<0>(geom_and_material)
                );

                if(cur_contact < least_contact && cur_contact.t > 1.0)
                {
                    least_contact = cur_contact;
                    least_material = std::get<1>(geom_and_material);
                }
            }

            for(std::size_t channel = 0; channel < 3; ++channel)
            {
                hdr_buf(
                    cur_col >> supersample_factor,
                    cur_row >> supersample_factor,
                    0,
                    channel
                ) += scene_materials[least_material][channel];
            }
        }
    }

    // Mostly-correct quantization, assuming we want to capture the
    // whole range of the generated image (no oversaturation).
    hdr_buf /= std::max(hdr_buf.max(), 1.0f);
    hdr_buf *= std::nextafter(256.0f, 0.0f);
    cimg::CImg<uint8_t> ldr_buf = hdr_buf;

    ldr_buf.save_jpeg("output.jpeg", 100);
}
