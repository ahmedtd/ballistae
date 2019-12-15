#include <iostream>

#include "include/libballistae/options.hh"
#include "include/libballistae/scene.hh"
#include "include/libballistae/render_scene.hh"
#include "include/libballistae/color.hh"
#include "include/libballistae/dense_signal.hh"

#include "include/libballistae/camera/pinhole.hh"

#include "include/libballistae/geometry/box.hh"
#include "include/libballistae/geometry/plane.hh"
#include "include/libballistae/geometry/infinity.hh"
#include "include/libballistae/geometry/sphere.hh"
#include "include/libballistae/geometry/surface_mesh.hh"

#include "include/libballistae/material/emitter.hh"
#include "include/libballistae/material/gauss.hh"
#include "include/libballistae/material/mc_lambert.hh"
#include "include/libballistae/material/pc_smooth.hh"

#include "include/libballistae/material_map.hh"

using namespace frustum;
using namespace ballistae;

int main(int argc, char **argv)
{
    options the_options = take_options(argc, argv);

    scene the_scene;

    auto cie_d65_emitter = materials::make_emitter(material_map::make_constant_spectrum(10 * cie_d65<double>()));
    auto cie_a_emitter = materials::make_emitter(material_map::make_constant_spectrum(10 * cie_a<double>()));
    auto matte = materials::make_gauss(material_map::make_constant_scalar(0.5));
    auto matte2 = materials::make_gauss(material_map::make_constant_scalar(0.0));

    infinity infinity;
    sphere sphere;

    surface_mesh bunny = surface_mesh_from_obj_file("bunny.obj", true);

    box center_box({span<double>{0, 0.5}, {0, 0.5}, {0, 0.5}});

    box ground({span<double>{0, 10.1},  span<double>{0, 10.1},  span<double>{-0.5, 0}});
    box   roof({span<double>{0, 10.1},  span<double>{0, 10.1},  span<double>{10, 10.1}});
    box wall_e({span<double>{10, 10.1}, span<double>{0, 10},    span<double>{0, 10}});
    box wall_n({span<double>{0, 10},    span<double>{10, 10.1}, span<double>{0, 10}});
    box wall_w({span<double>{-0.1, 0},  span<double>{0, 10},    span<double>{0, 10}});
    box wall_s({span<double>{0, 10},    span<double>{-0.1, 0},  span<double>{0, 10}});

    the_scene.elements = {
        {&infinity, &cie_d65_emitter, affine_transform<double, 3>::identity()},
        {&bunny, &cie_a_emitter, affine_transform<double, 3>::translation({5, 4, 0}) * affine_transform<double, 3>::scaling(10)},
        {&bunny, &matte, affine_transform<double, 3>::translation({4, 5, 0}) * affine_transform<double, 3>::scaling(10)},
        {&ground, &matte2, affine_transform<double, 3>::identity()},
        {&roof, &matte2, affine_transform<double, 3>::identity()},
        {&wall_n, &matte2, affine_transform<double, 3>::identity()},
        {&wall_w, &matte2, affine_transform<double, 3>::identity()},
        {&wall_s, &matte2, affine_transform<double, 3>::identity()}
    };

    crush(the_scene, 0.0);

    pinhole the_camera(
        {1 , 1, 2},
        {1, 0, 0, 0, 1, 0, 0, 0, 1},
        {0.02, 0.018, 0.012}
    );

    the_camera.set_eye(fixvec<double, 3>{5, 5, 1} - the_camera.center);

    render_scene(
        the_options,
        the_camera,
        the_scene,
        [](size_t cur, size_t tot) {
            std::cout << "\r" << cur << "/" << tot
            << " " << (cur / (tot / 100)) << "%" << std::flush;
        }
    );

    std::cout << std::endl;

    return 0;
}
