#ifndef BALLISTAE_MATERIAL_GAUSS_HH
#define BALLISTAE_MATERIAL_GAUSS_HH

#include <frustum-0/indicial/fixed.hh>

#include <libballistae/material.hh>
#include <libballistae/material_map.hh>
#include <libballistae/dense_signal.hh>
#include <libballistae/vector_distributions.hh>

namespace ballistae
{

namespace materials
{

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
        while(rejection_dist(g) < pdf_val);

        return candidate;
    }
};

template<class VarianceFn>
struct gauss : public material
{
    VarianceFn variance;

    gauss(VarianceFn variance_in)
        : variance(variance_in)
    {
    }

    virtual ~gauss()
    {
    }

    virtual void crush(const scene &the_scene, double time)
    {
    }

    virtual shade_info<double> shade(
        const scene &the_scene,
        const contact<double> &glb_contact,
        double lambda
    ) const
    {
        static thread_local std::mt19937 thread_rng;

        const auto &geom_p = glb_contact.p;
        const auto &refl_s = glb_contact.r.slope;
        const auto &geom_n = glb_contact.n;
        const auto &mtl2 = glb_contact.mtl2;
        const auto &mtl3 = glb_contact.mtl3;

        gaussian_dist<double, 3> facet_n_dist(
            geom_n,
            variance({mtl2, mtl3, lambda})
        );

        fixvec<double, 3> facet_n = facet_n_dist(thread_rng);
        // if(iprod(facet_n, refl_s) < 0.0)
        //     facet_n = geom_n;

        shade_info<double> result;
        result.emitted_power = 0.0;
        result.propagation_k = 1.0;
        result.incident_ray.point = geom_p;
        result.incident_ray.slope = reflect(refl_s, facet_n);

        return result;
    }
};

template<class VarianceFn>
auto make_gauss(VarianceFn variance)
{
    return gauss<VarianceFn>(variance);
}

}

}

#endif
