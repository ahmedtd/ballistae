#include <libballistae/camera_plugin_interface.hh>
#include <libguile_ballistae/camera_plugin_interface.hh>

#include <memory>

#include <armadillo>

#include <libballistae/vector.hh>

#include <libguile_armadillo/libguile_armadillo.hh>

class pinhole_priv : public ballistae::camera_priv
{
private:
    arma::vec3  center;
    arma::mat33 aperture_to_world;
    arma::vec3  aperture;
public:

    pinhole_priv(
        const arma::vec3  &center_in,
        const arma::mat33 &aperture_to_world_in,
        const arma::vec3 &aperture_in
    );

    ~pinhole_priv();

    virtual ballistae::dray3 image_to_ray(
        const arma::vec3 &image_coords
    ) const override;

    arma::vec3 eye() const;
    arma::vec3 left() const;
    arma::vec3 up() const;

    void set_center(const arma::vec3 &new_center);
    void set_eye(const arma::vec3 &new_eye);
    void set_up(const arma::vec3 &new_up);

    void set_aperture(const arma::vec3 &new_aperture);

    void set_eye_direct(const arma::vec3 &new_eye);
    void set_left_direct(const arma::vec3 &new_left);
    void set_up_direct(const arma::vec3 &new_up);
};

pinhole_priv::pinhole_priv(
    const arma::vec3  &center_in,
    const arma::mat33 &aperture_to_world_in,
    const arma::vec3 &aperture_in
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
    const arma::vec3 &image_coords
) const
{
    arma::vec3 aperture_coords = image_coords % aperture;
    return {center, arma::normalise(aperture_to_world * aperture_coords)};
}

arma::vec3 pinhole_priv::eye() const
{
    return aperture_to_world.col(0);
}

arma::vec3 pinhole_priv::left() const
{
    return aperture_to_world.col(1);
}

arma::vec3 pinhole_priv::up() const
{
    return aperture_to_world.col(2);
}

void pinhole_priv::set_center(const arma::vec3 &new_center)
{
    center = new_center;
}

void pinhole_priv::set_eye(const arma::vec3 &new_eye)
{
    set_eye_direct(arma::normalise(new_eye));
    set_up_direct(arma::normalise(ballistae::reject(this->eye(), this->up())));
    set_left_direct(arma::cross(this->up(), this->eye()));
}

void pinhole_priv::set_up(const arma::vec3 &new_up)
{
    set_up_direct(arma::normalise(ballistae::reject(this->eye(), new_up)));
    set_left_direct(arma::cross(this->up(), this->eye()));
}

void pinhole_priv::set_aperture(const arma::vec3 &new_aperture)
{
    aperture = new_aperture;
}

void pinhole_priv::set_eye_direct(const arma::vec3 &new_eye)
{
    aperture_to_world.unsafe_col(0) = new_eye;
}

void pinhole_priv::set_left_direct(const arma::vec3 &new_left)
{
    aperture_to_world.unsafe_col(1) = new_left;
}

void pinhole_priv::set_up_direct(const arma::vec3 &new_up)
{
    aperture_to_world.unsafe_col(2) = new_up;
}

std::shared_ptr<ballistae::camera_priv> ballistae_camera_create_from_alist(
    SCM config_alist
)
{
    constexpr const char* const subr = "ballistae_camera_create_from_alist(pinhole)";

    SCM sym_center       = scm_from_utf8_symbol("center");
    SCM sym_eye          = scm_from_utf8_symbol("eye");
    SCM sym_up           = scm_from_utf8_symbol("up");
    SCM sym_aperture_vec = scm_from_utf8_symbol("aperture-vec");

    SCM cur_tail = config_alist;
    while(cur_tail != SCM_EOL)
    {
        SCM cur_key = scm_caar(cur_tail);
        SCM cur_val = scm_cdar(cur_tail);
        cur_tail = scm_cdr(cur_tail);

        if(scm_is_true(scm_eq_p(sym_center, cur_key))
           || scm_is_true(scm_eq_p(sym_eye, cur_key))
           || scm_is_true(scm_eq_p(sym_up, cur_key))
           || scm_is_true(scm_eq_p(sym_aperture_vec, cur_key)))
        {
            SCM_ASSERT_TYPE(
                scm_is_true(
                    arma_guile::generic_col_dim_p<double>(
                        cur_val,
                        scm_from_int(3)
                    )
                ),
                cur_val,
                SCM_ARGn,
                subr,
                "arma/b64col[3]"
            );
        }
        else
        {
            scm_wrong_type_arg_msg(subr, SCM_ARGn, cur_key, "Unknown key.");
        }
    }

    // No guile errors below this point.

    auto pinhole_p = std::make_shared<pinhole_priv>(
        arma::vec3(arma::fill::zeros),
        arma::mat33(arma::fill::eye),
        arma::vec3(arma::fill::ones)
    );

    SCM center_lookup       = scm_assq_ref(config_alist, sym_center   );
    SCM eye_lookup          = scm_assq_ref(config_alist, sym_eye      );
    SCM up_lookup           = scm_assq_ref(config_alist, sym_up       );
    SCM aperture_vec_lookup = scm_assq_ref(config_alist, sym_aperture_vec);

    if(scm_is_true(center_lookup))
    {
        pinhole_p->set_center(arma_guile::extract_col<double>(center_lookup));
    }

    if(scm_is_true(eye_lookup))
    {
        pinhole_p->set_eye(arma_guile::extract_col<double>(eye_lookup));
    }

    if(scm_is_true(up_lookup))
    {
        pinhole_p->set_up(arma_guile::extract_col<double>(up_lookup));
    }

    if(scm_is_true(aperture_vec_lookup))
    {
        pinhole_p->set_aperture(
            arma_guile::extract_col<double>(aperture_vec_lookup)
        );
    }

    return pinhole_p;
}
