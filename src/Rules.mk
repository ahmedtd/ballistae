$(guile (augmk/enter "libballistae"))
$(guile (augmk/enter "libguile_ballistae"))

# Camera plugins
$(guile (augmk/enter "ballistae_camera_pinhole"))

# Geometry plugins
$(guile (augmk/enter "ballistae_geometry_cylinder"))
$(guile (augmk/enter "ballistae_geometry_infinity"))
$(guile (augmk/enter "ballistae_geometry_plane"))
$(guile (augmk/enter "ballistae_geometry_sphere"))
$(guile (augmk/enter "ballistae_geometry_surface_mesh"))

# Material plugins
$(guile (augmk/enter "ballistae_material_phong"))
$(guile (augmk/enter "ballistae_material_nonconductive_smooth"))
$(guile (augmk/enter "ballistae_material_mc_lambert"))
$(guile (augmk/enter "ballistae_material_omnidirectional_emitter"))
$(guile (augmk/enter "ballistae_material_directional_emitter"))
