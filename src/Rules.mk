$(guile (augmk/enter "libballistae"))
$(guile (augmk/enter "libguile_armadillo"))
$(guile (augmk/enter "libguile_ballistae"))

# Camera plugins
$(guile (augmk/enter "ballistae_camera_plugin_pinhole"))

# Geometry plugins
$(guile (augmk/enter "ballistae_geom_plugin_cylinder"))
$(guile (augmk/enter "ballistae_geom_plugin_plane"))
$(guile (augmk/enter "ballistae_geom_plugin_sphere"))

# Material plugins
$(guile (augmk/enter "ballistae_matr_plugin_phong"))
