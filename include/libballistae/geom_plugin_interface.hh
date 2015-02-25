#ifndef LIBBALLISTAE_GEOM_PLUGIN_INTERFACE_HH
#define LIBBALLISTAE_GEOM_PLUGIN_INTERFACE_HH

////////////////////////////////////////////////////////////////////////////////
/// The interface that geometry plugins should implement.
////////////////////////////////////////////////////////////////////////////////
///
/// libballistae requires these functions to be present for a shared object to
/// function as a geometry plugin.  Note the absence of any way to create the
/// geometry --- this task is deferred to cooperation between the front-end and
/// the plugin.
///
/// In practice, each plugin will also provide a function
/// [ballistae_geom_create_from_alist], which takes a GNU guile SCM alist
/// containing initialization parameters.  However, I am of the opinion that
/// libballistae should not _force_ guile integration.

#include <cstddef>

#include <armadillo>

#include <libballistae/ray.hh>
#include <libballistae/span.hh>

namespace ballistae
{

class geom_priv
{
public:
    virtual ~geom_priv() {}

    /// Ray intersection (batch interface).
    ///
    /// Write the [index]th entry of ray query[i] that is within the span
    /// [must_overlap] into out_src[2*i], and the corresponding exit into
    /// out_src[2*i+1]
    virtual void ray_intersect(
        const dray3 *query_src,
        const dray3 *query_lim,
        const span<double> &must_overlap,
        const std::size_t index,
        span<double> *out_spans_src,
        arma::vec3 *out_normals_src
    ) const = 0;
};

}

#endif
