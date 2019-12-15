#ifndef BALLISTAE_CAMERA_PINHOLE_HH
#define BALLISTAE_CAMERA_PINHOLE_HH

#include <random>

#include "include/libballistae/camera.hh"
#include "include/libballistae/vector.hh"

#include "include/frustum-0/indicial/fixed.hh"

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
    ) : center(center_in),
        aperture_to_world(aperture_to_world_in),
        aperture(aperture_in)
    {
    }

    ~pinhole()
    {
    }

    virtual dray3 image_to_ray(
        const fixvec<double, 3> &image_coords,
        std::mt19937 &rng
    ) const override
    {
        using frustum::eltwise_mul;
        using frustum::normalise;

        fixvec<double, 3> aperture_coords = eltwise_mul(image_coords, aperture);
        return {center, normalise(aperture_to_world * aperture_coords)};
    }

    fixvec<double, 3> eye() const
    {
        fixvec<double, 3> col = {
            aperture_to_world(0,0),
            aperture_to_world(1,0),
            aperture_to_world(2,0)
        };
        return col;
    }

    fixvec<double, 3> left() const
    {
        fixvec<double, 3> col = {
            aperture_to_world(0,1),
            aperture_to_world(1,1),
            aperture_to_world(2,1)
        };
        return col;
    }

    fixvec<double, 3> up() const
    {
        fixvec<double, 3> col = {
            aperture_to_world(0,2),
            aperture_to_world(1,2),
            aperture_to_world(2,2)
        };
        return col;
    }

    void set_center(const fixvec<double, 3> &new_center)
    {
        center = new_center;
    }

    void set_eye(const fixvec<double, 3> &new_eye)
    {
        set_eye_direct(normalise(new_eye));
        set_up_direct(normalise(reject(eye(), up())));
        set_left_direct(cprod(this->up(), this->eye()));
    }

    void set_up(const fixvec<double, 3> &new_up)
    {
        set_up_direct(normalise(reject(this->eye(), new_up)));
        set_left_direct(cprod(this->up(), this->eye()));
    }

    void set_aperture(const fixvec<double, 3> &new_aperture)
    {
        aperture = new_aperture;
    }

    void set_eye_direct(const fixvec<double, 3> &new_eye)
    {
        aperture_to_world(0,0) = new_eye(0);
        aperture_to_world(1,0) = new_eye(1);
        aperture_to_world(2,0) = new_eye(2);
    }

    void set_left_direct(const fixvec<double, 3> &new_left)
    {
        aperture_to_world(0,1) = new_left(0);
        aperture_to_world(1,1) = new_left(1);
        aperture_to_world(2,1) = new_left(2);
    }

    void set_up_direct(const fixvec<double, 3> &new_up)
    {
        aperture_to_world(0,2) = new_up(0);
        aperture_to_world(1,2) = new_up(1);
        aperture_to_world(2,2) = new_up(2);
    }
};

}

#endif
