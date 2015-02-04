#ifndef BALLISTAE_GEOMETRY_PLUGIN_HH
#define BALLISTAE_GEOMETRY_PLUGIN_HH

/// The plugin's per-instance data.
///
/// Each plugin should provide its own definition of this structure.  The main
/// ballistae executable only handles this data type via pointer.
struct geom_priv;

extern "C" geom_priv* ballistae_geom_create(
    const std::vector<std::tuple<string, string>> &init_data
) nothrow;

extern "C" std::tuple<double, arma::vec3, arma::vec3>
ballistae_geom_ray_intersect(
    const ballistae::ray &query,
    geom_priv *data
);

extern "C" geom_priv* ballistae_geom_destroy(
    geom_priv *ptr
) nothrow;

#endif
