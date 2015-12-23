example_srcs := $(wildcard $(augmk_d)/*.cc)
example_bins := $(example_srcs:.cc=)

CLEAN_TARGETS += $(example_bins)
ALL_TARGETS   += $(example_bins)

example_CXXFLAGS :=
example_CXXFLAGS += -Iinclude

example_LFLAGS :=
example_LFLAGS += -L src/libballistae -lballistae
example_LFLAGS += -lboost_program_options

$(guile (augmk/gcc/autodep-cc "$(example_srcs)" "$(example_bins)" "$(example_CXXFLAGS) $(CXXFLAGS)"))

$(example_bins) : % : %.cc
	$(CXX) -o $@ $< $(example_CXXFLAGS) $(example_LFLAGS) $(CXXFLAGS)
