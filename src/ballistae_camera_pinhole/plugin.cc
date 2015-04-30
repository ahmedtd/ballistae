#include <libballistae/camera.hh>
#include <libguile_ballistae/camera_plugin_interface.hh>

#include <random>

#include <libballistae/vector.hh>

#include <frustum-0/indicial/fixed.hh>

#include <libguile_frustum0/libguile_frustum0.hh>

using namespace frustum;
using namespace ballistae;

class pinhole_priv : public ballistae::camera
{
private:
    fixvec<double, 3>  center;
    fixmat<double, 3, 3> aperture_to_world;
    fixvec<double, 3>  aperture;
public:

    pinhole_priv(
        const fixvec<double, 3>  &center_in,
        const fixmat<double, 3, 3> &aperture_to_world_in,
        const fixvec<double, 3> &aperture_in
    );

    ~pinhole_priv();

    virtual dray3 image_to_ray(
        const fixvec<double, 3> &image_coords,
        std::ranlux24 &rng
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

pinhole_priv::pinhole_priv(
    const fixvec<double, 3>  &center_in,
    const fixmat<double, 3, 3> &aperture_to_world_in,
    const fixvec<double, 3> &aperture_in
)
    : center(center_in),
      aperture_to_world(aperture_to_world_in),
      aperture(aperture_in)
{
}

pinhole_priv::~pinhole_priv()
{
}

ballistae::dray3 pinhole_priv::image_to_ray(
    const fixvec<double, 3> &image_coords,
    std::ranlux24 &rng
) const
{
    using frustum::eltwise_mul;
    using frustum::normalise;

    fixvec<double, 3> aperture_coords = eltwise_mul(image_coords, aperture);
    return {center, normalise(aperture_to_world * aperture_coords)};
}

fixvec<double, 3> pinhole_priv::eye() const
{
    fixvec<double, 3> col = {
        aperture_to_world(0,0),
        aperture_to_world(1,0),
        aperture_to_world(2,0)
    };
    return col;
}

fixvec<double, 3> pinhole_priv::left() const
{
    fixvec<double, 3> col = {
        aperture_to_world(0,1),
        aperture_to_world(1,1),
        aperture_to_world(2,1)
    };
    return col;
}

fixvec<double, 3> pinhole_priv::up() const
{
    fixvec<double, 3> col = {
        aperture_to_world(0,2),
        aperture_to_world(1,2),
        aperture_to_world(2,2)
    };
    return col;
}

void pinhole_priv::set_center(const fixvec<double, 3> &new_center)
{
    center = new_center;
}

void pinhole_priv::set_eye(const fixvec<double, 3> &new_eye)
{
    set_eye_direct(normalise(new_eye));
    set_up_direct(normalise(reject(eye(), up())));
    set_left_direct(cprod(this->up(), this->eye()));
}

void pinhole_priv::set_up(const fixvec<double, 3> &new_up)
{
    set_up_direct(normalise(reject(this->eye(), new_up)));
    set_left_direct(cprod(this->up(), this->eye()));
}

void pinhole_priv::set_aperture(const fixvec<double, 3> &new_aperture)
{
    aperture = new_aperture;
}

void pinhole_priv::set_eye_direct(const fixvec<double, 3> &new_eye)
{
    aperture_to_world(0,0) = new_eye(0);
    aperture_to_world(1,0) = new_eye(1);
    aperture_to_world(2,0) = new_eye(2);
}

void pinhole_priv::set_left_direct(const fixvec<double, 3> &new_left)
{
    aperture_to_world(0,1) = new_left(0);
    aperture_to_world(1,1) = new_left(1);
    aperture_to_world(2,1) = new_left(2);
}

void pinhole_priv::set_up_direct(const fixvec<double, 3> &new_up)
{
    aperture_to_world(0,2) = new_up(0);
    aperture_to_world(1,2) = new_up(1);
    aperture_to_world(2,2) = new_up(2);
}

ballistae::camera* guile_ballistae_camera(
    SCM config_alist
)
{
    SCM sym_center       = scm_from_utf8_symbol("center");
    SCM sym_eye          = scm_from_utf8_symbol("eye");
    SCM sym_up           = scm_from_utf8_symbol("up");
    SCM sym_aperture_vec = scm_from_utf8_symbol("aperture-vec");

    auto pinhole_p = new pinhole_priv(
        fixvec<double, 3>::zero(),
        fixmat<double, 3, 3>::eye(),
        {1, 1, 1}
    );

    SCM center_lookup       = scm_assq_ref(config_alist, sym_center   );
    SCM eye_lookup          = scm_assq_ref(config_alist, sym_eye      );
    SCM up_lookup           = scm_assq_ref(config_alist, sym_up       );
    SCM aperture_vec_lookup = scm_assq_ref(config_alist, sym_aperture_vec);

    if(scm_is_true(center_lookup))
    {
        pinhole_p->set_center(guile_frustum::dvec3_from_scm(center_lookup));
    }

    if(scm_is_true(eye_lookup))
    {
        pinhole_p->set_eye(guile_frustum::dvec3_from_scm(eye_lookup));
    }

    if(scm_is_true(up_lookup))
    {
        pinhole_p->set_up(guile_frustum::dvec3_from_scm(up_lookup));
    }

    if(scm_is_true(aperture_vec_lookup))
    {
        pinhole_p->set_aperture(
            guile_frustum::dvec3_from_scm(aperture_vec_lookup)
        );
    }

    return pinhole_p;
}
