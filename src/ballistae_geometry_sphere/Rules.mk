sphere_plugin_srcs := $(wildcard $(augmk_d)/*.cc)
sphere_plugin_objs := $(sphere_plugin_srcs:.cc=.o)

CLEAN_TARGETS += $(sphere_plugin_objs)

sphere_plugin_CXXFLAGS :=
sphere_plugin_CXXFLAGS += -fPIC
sphere_plugin_CXXFLAGS += $(armadillo4_CFLAGS)
sphere_plugin_CXXFLAGS += -Iinclude
sphere_plugin_CXXFLAGS += $(guile2_CFLAGS)

sphere_plugin_LFLAGS :=
sphere_plugin_LFLAGS += -Lsrc/libballistae/ -lballistae
sphere_plugin_LFLAGS += -Lsrc/libguile_armadillo/ -lguile_armadillo
sphere_plugin_LFLAGS += $(armadillo4_LIBS)
sphere_plugin_LFLAGS += $(guile2_LIBS)

# Load automatically-generated dependencies.
$(guile (augmk/gcc/autodep-cc "$(sphere_plugin_srcs)" "$(sphere_plugin_objs)" \
    "$(sphere_plugin_CXXFLAGS) $(CXXFLAGS)"))

$(sphere_plugin_objs): %.o : %.cc
	$(CXX) -c -o $@ $< $(sphere_plugin_CXXFLAGS) $(CXXFLAGS)

$(augmk_d)/ballistae_geometry_sphere.so : | src/libballistae/libballistae.so
$(augmk_d)/ballistae_geometry_sphere.so : | src/libguile_armadillo/libguile_armadillo.so
$(augmk_d)/ballistae_geometry_sphere.so : $(sphere_plugin_objs)
	$(CXX) -shared -o $@ $^ $(sphere_plugin_CXXFLAGS) $(sphere_plugin_LFLAGS) $(CXXFLAGS)

ALL_TARGETS += $(augmk_d)/ballistae_geometry_sphere.so
CLEAN_TARGETS += $(augmk_d)/ballistae_geometry_sphere.so

INSTALL_TARGETS += $(guile (augmk/install-lib-phony "$(augmk_d)_install" "./" "$(augmk_d)/ballistae_geometry_sphere.so"))
