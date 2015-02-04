#ifndef BALLISTAE_CAMERA_HH
#define BALLISTAE_CAMERA_HH

#include <armadillo>

#include <ballistae/ray.hh>

namespace ballistae
{

struct camera
{
    arma::vec3 loc;

    arma::mat33 aperture_to_world;

    arma::vec3 aperture;

    camera(
        const arma::vec3 &loc_in,
        const arma::vec3 &eye_in,
        const arma::vec3 &up_in,
        const arma::vec3 &aperture_in
    );

    arma::vec3 eye() const;
    arma::vec3 left() const;
    arma::vec3 up() const;

    void set_eye(const arma::vec3 &new_eye);
    void set_up(const arma::vec3 &new_up);

    void set_eye_direct(const arma::vec3 &new_eye);
    void set_left_direct(const arma::vec3 &new_left);
    void set_up_direct(const arma::vec3 &new_up);
};

dray3 camera_ray(
    const camera &cam,
    const arma::vec3 &image_coords
) noexcept __attribute__((pure));


}

#endif
