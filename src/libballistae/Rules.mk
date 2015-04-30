libballistae_srcs := $(wildcard $(augmk_d)/*.cc)
libballistae_objs := $(libballistae_srcs:.cc=.o)

CLEAN_TARGETS += $(libballistae_objs)

libballistae_CXXFLAGS :=
libballistae_CXXFLAGS += -fopenmp
libballistae_CXXFLAGS += -fPIC
libballistae_CXXFLAGS += $(frustum0_CFLAGS)
libballistae_CXXFLAGS += $(libguile_frustum0_CFLAGS)
libballistae_CXXFLAGS += -Iinclude

libballistae_LFLAGS :=
libballistae_LFLAGS += $(frustum0_LIBS)
libballistae_LFLAGS += $(libguile_frustum0_LIBS)

# Load auto dependencies
$(guile (augmk/gcc/autodep-cc "$(libballistae_srcs)" \
							  "$(libballistae_objs)" \
							  "$(libballistae_CXXFLAGS) $(CXXFLAGS)"))

$(libballistae_objs) : %.o : %.cc
	$(CXX) -c -o $@ $< $(libballistae_CXXFLAGS) $(CXXFLAGS)

$(augmk_d)/libballistae.so : $(libballistae_objs)
	$(CXX) -shared -o $@ $^ $(libballistae_CXXFLAGS) $(libballistae_LFLAGS) $(CXXFLAGS)

ALL_TARGETS += $(augmk_d)/libballistae.so
CLEAN_TARGETS += $(augmk_d)/libballistae.so

INSTALL_TARGETS += $(guile (augmk/install-lib-phony "$(augmk_d)/libballistae.so_install" "./" "$(augmk_d)/libballistae.so"))
