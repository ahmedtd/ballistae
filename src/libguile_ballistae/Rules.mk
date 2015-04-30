libguile_ballistae_srcs := $(wildcard $(augmk_d)/*.cc)
libguile_ballistae_objs := $(libguile_ballistae_srcs:.cc=.o)

CLEAN_TARGETS += $(libguile_ballistae_objs)

libguile_ballistae_CXXFLAGS :=
libguile_ballistae_CXXFLAGS += -fPIC -fvisibility=hidden
libguile_ballistae_CXXFLAGS += $(frustum0_CFLAGS)
libguile_ballistae_CXXFLAGS += $(libguile_frustum0_CFLAGS)
libguile_ballistae_CXXFLAGS += $(guile2_CFLAGS)
libguile_ballistae_CXXFLAGS += -Iinclude
libguile_ballistae_CXXFLAGS += -pthread # We use c++14 threads.

libguile_ballistae_LFLAGS :=
libguile_ballistae_LFLAGS += $(frustum0_LIBS)
libguile_ballistae_LFLAGS += $(libguile_frustum0_LIBS)
libguile_ballistae_LFLAGS += $(guile2_LIBS)
libguile_ballistae_LFLAGS += -Lsrc/libballistae/ -lballistae

# Load auto dependencies
$(guile (augmk/gcc/autodep-cc "$(libguile_ballistae_srcs)" \
							  "$(libguile_ballistae_objs)" \
							  "$(libguile_ballistae_CXXFLAGS) $(CXXFLAGS)"))

$(libguile_ballistae_objs) : %.o : %.cc
	$(CXX) -c -o $@ $< $(libguile_ballistae_CXXFLAGS) $(CXXFLAGS)

# The dependency on libballistae is order-only to prevent it from being part of
# the $^ argument, and thus getting linked into the plugin library.
$(augmk_d)/libguile_ballistae.so : | src/libballistae/libballistae.so
$(augmk_d)/libguile_ballistae.so : $(libguile_ballistae_objs)
	$(CXX) -shared -o $@ $^ $(libguile_ballistae_CXXFLAGS) $(libguile_ballistae_LFLAGS) $(CXXFLAGS)

ALL_TARGETS += $(augmk_d)/libguile_ballistae.so
CLEAN_TARGETS += $(augmk_d)/libguile_ballistae.so

INSTALL_TARGETS += $(guile (augmk/install-lib-phony "$(augmk_d)_install" "./" "$(augmk_d)/libguile_ballistae.so"))
