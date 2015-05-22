#include <libballistae/material.hh>
#include <libguile_ballistae/material_plugin_interface.hh>

#include <memory>
#include <random>

#include <frustum-0/indicial/fixed.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

#include <libballistae/material_map.hh>
#include <libballistae/dense_signal.hh>
#include <libballistae/vector_distributions.hh>

using namespace frustum;
using namespace ballistae;

template<class Field, size_t D>
struct gaussian_dist
{
    uniform_unitv_distribution<Field, D> backing_dist;
    fixvec<Field, D> normal;
    Field variance;
    /// Rejection distribution.
    std::uniform_real_distribution<Field> rejection_dist;

    gaussian_dist(
        const fixvec<Field, D> normal_in,
        const Field &variance_in
    )
        : normal(normal_in),
          variance(variance_in),
          rejection_dist(0, 1)
    {
    }

    template<class Gen>
    fixvec<Field, D> operator()(Gen &g)
    {
        using std::exp;

        // If the variance is 0, then the pdf is a delta function centered at 0,
        // which means only the normal would be returned.
        if(variance == Field(0))
            return normal;

        // By cutting the case variance == 0 from the rejection testing, we
        // ensure that the loop below will terminate, since the result of exp
        // will never be NaN.

        // The loop could, however, take a very long time indeed for small
        // variance values, since the pdf evaluates to zero for larger and
        // larger fractions of the rejection test interval as v gets larger.

        // In general, for double precision arithmetic, truly problematic values
        // will start as v gets smaller than ~(1/1400).

        fixvec<Field, D> candidate;
        Field pdf_val;
        do
        {
            candidate = backing_dist(g);
            Field cosine = iprod(normal, candidate);
            
            if(cosine < Field(0))
            {
                candidate = -candidate;
                cosine = -cosine;
            }

            // We neglect the leading normalization factor, since we can then
            // just use rejection values in [0,1] without changing anything.
            pdf_val = exp(-cosine * cosine / (Field(2) * variance));
        }
        while(pdf_val != Field(0) && rejection_dist(g) < pdf_val);

        // If the exponential argument blew up to infinity, then just return
        // the normal, since only values very close to the normal would be
        // accepted anyways.
        if(pdf_val == Field(0))
            return normal;
        else
            return candidate;
    }
};

struct material_gauss : public ballistae_guile::updatable_material
{
    mtlmap<1> *variance;

    material_gauss();
    virtual ~material_gauss();

    virtual shade_info<double> shade(
        const scene &the_scene,
        const contact<double> &glb_contact,
        double lambda_src,
        double lambda_lim,
        double lambda_cur,
        size_t sample_index,
        std::ranlux24 &thread_rng
    ) const;

    virtual void guile_update(scene *p_scene, SCM config);
};

material_gauss::material_gauss()
{
}

material_gauss::~material_gauss()
{
}

shade_info<double> material_gauss::shade(
    const scene &the_scene,
    const contact<double> &glb_contact,
    double lambda_src,
    double lambda_lim,
    double lambda_cur,
    size_t sample_index,
    std::ranlux24 &thread_rng
) const
{
    const auto &geom_p = glb_contact.p;
    const auto &refl_s = glb_contact.r.slope;
    const auto &geom_n = glb_contact.n;
    const auto &mtl2 = glb_contact.mtl2;
    const auto &mtl3 = glb_contact.mtl3;

    gaussian_dist<double, 3> facet_n_dist(
        geom_n,
        variance->value(mtl2, mtl3, lambda_cur)(0)
    );
    
    auto incident_slope = refl_s;
    while(iprod(incident_slope, geom_n) < 0.0)
    {
        // It's impossible to hit the backside of a microfacet.
        fixvec<double, 3> facet_n;
        do
        {
            facet_n = facet_n_dist(thread_rng);
        }
        while(iprod(incident_slope, facet_n) > 0.0);

        incident_slope = reflect(incident_slope, facet_n);
    }

    shade_info<double> result;
    result.emitted_power = 0.0;
    result.propagation_k = 1.0;
    result.incident_ray.point = geom_p;
    result.incident_ray.slope = incident_slope;

    return result;
}

void material_gauss::guile_update(scene *p_scene, SCM config)
{
    SCM sym_variance = scm_from_utf8_symbol("variance");
    SCM lu_variance = scm_assq_ref(config, sym_variance);

    if(scm_is_true(lu_variance))
        this->variance = p_scene->mtlmaps_1[scm_to_size_t(lu_variance)].get();
}

std::unique_ptr<ballistae_guile::updatable_material>
guile_ballistae_material(scene *p_scene, SCM config)
{
    using namespace ballistae_guile;

    auto p = std::make_unique<material_gauss>();

    // mtlmap 0 is *always* a constant smits_white mtlmap.
    p->variance = p_scene->mtlmaps_1[0].get();
    p->guile_update(p_scene, config);

    return std::move(p);
}
