ballistae_srcs := $(wildcard $(augmk_d)/*.cc)
ballistae_objs := $(ballistae_srcs:.cc=.o)

CLEAN_TARGETS += $(ballistae_objs)

ballistae_CXXFLAGS :=
ballistae_CXXFLAGS += $(armadillo4_CFLAGS)
ballistae_CXXFLAGS += $(guile2_CFLAGS)
ballistae_CXXFLAGS += -Iinclude

ballistae_LFLAGS :=
ballistae_LFLAGS += $(guile2_LIBS)
ballistae_LFLAGS += $(armadillo4_LIBS)

# Load auto dependencies.
$(guile (augmk/gcc/autodep-cc "$(ballistae_srcs)" "$(ballistae_objs)" "$(ballistae_CXXFLAGS) $(CXXFLAGS)"))

$(ballistae_objs) : %.o : %.cc
	g++ -c -o $@ $< $(ballistae_CXXFLAGS) $(CXXFLAGS)

$(augmk_d)/ballistae: $(ballistae_objs)
	g++ -o $@ $^ $(ballistae_CXXFLAGS) $(BALLISTAE_LFLAGS) $(CXXFLAGS)

ALL_TARGETS += $(augmk_d)/ballistae
CLEAN_TARGETS += $(augmk_d)/ballistae
