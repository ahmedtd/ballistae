#ifndef LIBGUILE_BALLISTAE_GEOM_INSTANCE_HH
#define LIBGUILE_BALLISTAE_GEOM_INSTANCE_HH

#include <memory>
#include <vector>

#include <libballistae/affine_transform.hh>
#include <libballistae/geometry.hh>

#include <libguile_ballistae/libguile_ballistae.hh>

namespace ballistae_guile
{

namespace geometry
{

subsmob_fns init();

////////////////////////////////////////////////////////////////////////////////
/// Make a geometry subsmob
////////////////////////////////////////////////////////////////////////////////
SCM make_backend(SCM plug_soname, SCM config_alist);

////////////////////////////////////////////////////////////////////////////////
/// Get the wrapped pointer.
////////////////////////////////////////////////////////////////////////////////
ballistae::geometry* p_from_scm(SCM geom);

}

}

#endif
