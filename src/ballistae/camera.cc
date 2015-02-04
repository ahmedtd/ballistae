#include <ballistae/camera.hh>

#include <armadillo>

#include <ballistae/ray.hh>
#include <ballistae/vector.hh>

namespace ballistae
{

camera::camera(
        const arma::vec3 &loc_in,
        const arma::vec3 &eye_in,
        const arma::vec3 &up_in,
        const arma::vec3 &aperture_in
)
    : loc(loc_in),
      aperture_to_world(arma::fill::eye),
      aperture(aperture_in)
{
    set_eye(eye_in);
    set_up(up_in);
}

arma::vec3 camera::eye() const
{
    return aperture_to_world.col(0);
}

arma::vec3 camera::left() const
{
    return aperture_to_world.col(1);
}

arma::vec3 camera::up() const
{
    return aperture_to_world.col(2);
}

void camera::set_eye(const arma::vec3 &new_eye)
{
    set_eye_direct(arma::normalise(new_eye));
    set_up_direct(arma::normalise(reject(this->eye(), this->up())));
    set_left_direct(arma::cross(this->up(), this->eye()));
}

void camera::set_up(const arma::vec3 &new_up)
{
    set_up_direct(arma::normalise(reject(this->eye(), new_up)));
    set_left_direct(arma::cross(this->up(), this->eye()));
}

void camera::set_eye_direct(const arma::vec3 &new_eye)
{
    aperture_to_world.unsafe_col(0) = new_eye;
}

void camera::set_left_direct(const arma::vec3 &new_left)
{
    aperture_to_world.unsafe_col(1) = new_left;
}

void camera::set_up_direct(const arma::vec3 &new_up)
{
    aperture_to_world.unsafe_col(2) = new_up;
}

dray3 camera_ray(
    const camera &cam,
    const arma::vec3 &image_coords
)
    noexcept
{
    arma::vec3 aperture_coords = image_coords % cam.aperture;

    dray3 ray;
    ray.point = cam.loc;
    ray.slope = cam.aperture_to_world * aperture_coords;

    ray.slope = arma::normalise(ray.slope);

    return ray;
}

}
