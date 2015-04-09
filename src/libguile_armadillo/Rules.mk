$(augmk_d)_srcs := $(wildcard $(augmk_d)/*.cc)
$(augmk_d)_objs := $($(augmk_d)_srcs:.cc=.o)

CLEAN_TARGETS += $($(augmk_d)_objs)

$(augmk_d)_CXXFLAGS :=
$(augmk_d)_CXXFLAGS += -fPIC
$(augmk_d)_CXXFLAGS += $(armadillo4_CFLAGS)
$(augmk_d)_CXXFLAGS += $(guile2_CFLAGS)
$(augmk_d)_CXXFLAGS += -Iinclude

$(augmk_d)_LFLAGS :=
$(augmk_d)_LFLAGS += $(armadillo4_LIBS)
$(augmk_d)_LFLAGS += $(guile2_LIBS)

# Load auto dependencies
$(guile (augmk/gcc/autodep-cc "$($(augmk_d)_srcs)" \
							  "$($(augmk_d)_objs)" \
							  "$($(augmk_d)_CXXFLAGS) $(CXXFLAGS)"))

$($(augmk_d)_objs) : d := $(augmk_d)
$($(augmk_d)_objs) : %.o : %.cc
	$(CXX) -c -o $@ $< $($(d)_CXXFLAGS) $(CXXFLAGS)

$(augmk_d)/libguile_armadillo.so : d := $(augmk_d)
$(augmk_d)/libguile_armadillo.so : $($(augmk_d)_objs)
	$(CXX) -shared -o $@ $^ $($(d)_CXXFLAGS) $($(d)_LFLAGS) $(CXXFLAGS)

ALL_TARGETS += $(augmk_d)/libguile_armadillo.so
CLEAN_TARGETS += $(augmk_d)/libguile_armadillo.so

INSTALL_TARGETS += $(guile (augmk/install-lib-phony "$(augmk_d)_install" "./" "$(augmk_d)/libguile_armadillo.so"))
