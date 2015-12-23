#ifndef BALLISTAE_CAMERA_PINHOLE_HH
#define BALLISTAE_CAMERA_PINHOLE_HH

#include <libballistae/camera.hh>

#include <frustum-0/indicial/fixed.hh>

namespace ballistae
{

class pinhole : public ballistae::camera
{
public:
    fixvec<double, 3>  center;
    fixmat<double, 3, 3> aperture_to_world;
    fixvec<double, 3>  aperture;

    pinhole(
        const fixvec<double, 3>  &center_in,
        const fixmat<double, 3, 3> &aperture_to_world_in,
        const fixvec<double, 3> &aperture_in
    );

    ~pinhole();

    virtual dray3 image_to_ray(
        const fixvec<double, 3> &image_coords,
        std::mt19937 &rng
    ) const override;

    fixvec<double, 3> eye() const;
    fixvec<double, 3> left() const;
    fixvec<double, 3> up() const;

    void set_center(const fixvec<double, 3> &new_center);
    void set_eye(const fixvec<double, 3> &new_eye);
    void set_up(const fixvec<double, 3> &new_up);

    void set_aperture(const fixvec<double, 3> &new_aperture);

    void set_eye_direct(const fixvec<double, 3> &new_eye);
    void set_left_direct(const fixvec<double, 3> &new_left);
    void set_up_direct(const fixvec<double, 3> &new_up);
};

}

#endif
