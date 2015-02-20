$(augmk_d)_srcs := $(wildcard $(augmk_d)/*.cc)
$(augmk_d)_objs := $($(augmk_d)_srcs:.cc=.o)

CLEAN_TARGETS += $($(augmk_d)_objs)

$(augmk_d)_CXXFLAGS :=
$(augmk_d)_CXXFLAGS += -fPIC
$(augmk_d)_CXXFLAGS += $(armadillo4_CFLAGS)
$(augmk_d)_CXXFLAGS += -Iinclude
$(augmk_d)_CXXFLAGS += $(guile2_CFLAGS)

$(augmk_d)_LFLAGS :=
$(augmk_d)_LFLAGS += -Lsrc/libballistae/ -lballistae
$(augmk_d)_LFLAGS += -Lsrc/libguile_armadillo/ -lguile_armadillo
$(augmk_d)_LFLAGS += $(armadillo4_LIBS)
$(augmk_d)_LFLAGS += $(guile2_LIBS)

# Load automatically-generated dependencies.
$(guile (augmk/gcc/autodep-cc "$($(augmk_d)_srcs)" "$($(augmk_d)_objs)" \
	"$($(augmk_d)_CXXFLAGS) $(CXXFLAGS)"))

$($(augmk_d)_objs): d := $(augmk_d)
$($(augmk_d)_objs): %.o : %.cc
	g++ -c -o $@ $< $($(d)_CXXFLAGS) $(CXXFLAGS)

$(augmk_d)/ballistae_geom_plugin_cylinder.so : d := $(augmk_d)
$(augmk_d)/ballistae_geom_plugin_cylinder.so : | src/libballistae/libballistae.so
$(augmk_d)/ballistae_geom_plugin_cylinder.so : | src/libguile_armadillo/libguile_armadillo.so
$(augmk_d)/ballistae_geom_plugin_cylinder.so : $($(augmk_d)_objs)
	g++ -shared -o $@ $^ $($(d)_CXXFLAGS) $($(d)_LFLAGS) $(CXXFLAGS)

ALL_TARGETS   += $(augmk_d)/ballistae_geom_plugin_cylinder.so
CLEAN_TARGETS += $(augmk_d)/ballistae_geom_plugin_cylinder.so