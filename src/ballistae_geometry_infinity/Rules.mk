$(augmk_d)_srcs := $(wildcard $(augmk_d)/*.cc)
$(augmk_d)_objs := $($(augmk_d)_srcs:.cc=.o)

CLEAN_TARGETS += $($(augmk_d)_objs)

$(augmk_d)_CXXFLAGS :=
$(augmk_d)_CXXFLAGS += -fPIC
$(augmk_d)_CXXFLAGS += $(frustum0_CFLAGS)
$(augmk_d)_CXXFLAGS += $(libguile_frustum0_CFLAGS)
$(augmk_d)_CXXFLAGS += -Iinclude
$(augmk_d)_CXXFLAGS += $(guile2_CFLAGS)

$(augmk_d)_LFLAGS :=
$(augmk_d)_LFLAGS += -Lsrc/libballistae/ -lballistae
$(augmk_d)_LFLAGS += $(frustum0_LIBS)
$(augmk_d)_LFLAGS += $(libguile_frustum0_LIBS)
$(augmk_d)_LFLAGS += $(guile2_LIBS)

# Load automatically-generated dependencies.
$(guile (augmk/gcc/autodep-cc "$($(augmk_d)_srcs)" "$($(augmk_d)_objs)" \
	"$($(augmk_d)_CXXFLAGS) $(CXXFLAGS)"))

$($(augmk_d)_objs): d := $(augmk_d)
$($(augmk_d)_objs): %.o : %.cc
	$(CXX) -c -o $@ $< $($(d)_CXXFLAGS) $(CXXFLAGS)

# The dependency on libballistae is order-only to prevent it from part of the $^
# argument, and thus getting linked into the plugin library.
$(augmk_d)/ballistae_geometry_infinity.so : d := $(augmk_d)
$(augmk_d)/ballistae_geometry_infinity.so : | src/libballistae/libballistae.so
$(augmk_d)/ballistae_geometry_infinity.so : $($(augmk_d)_objs)
	$(CXX) -shared -o $@ $^ $($(d)_CXXFLAGS) $($(d)_LFLAGS) $(CXXFLAGS)

ALL_TARGETS   += $(augmk_d)/ballistae_geometry_infinity.so
CLEAN_TARGETS += $(augmk_d)/ballistae_geometry_infinity.so

INSTALL_TARGETS += $(guile (augmk/install-lib-phony "$(augmk_d)_install" "./" "$(augmk_d)/ballistae_geometry_infinity.so"))
