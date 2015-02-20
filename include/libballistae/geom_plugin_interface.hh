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

#include <libballistae/contact.hh>
#include <libballistae/ray.hh>

namespace ballistae
{

class geom_priv
{
public:
    virtual ~geom_priv() {}

    virtual contact ray_intersect(
        const dray3 &query
    ) const = 0;
};

}

#endif
