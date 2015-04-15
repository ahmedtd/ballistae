CC  := gcc
CXX := g++

# We use C++14, with GNU extensions, across the board.
override CXXFLAGS += -std=gnu++1y -Wall -Werror -Wl,--no-undefined -fno-signed-zeros -fno-trapping-math -fno-rounding-math -fassociative-math -Wno-free-nonheap-object

# Find armadillo.
#
# We don't actually find it.
armadillo4_CFLAGS := -DARMA_MAT_PREALLOC=3
armadillo4_LIBS   := -larmadillo

GUILE_INSTALL_VERSION := 2.0

# Find guile 2.
#
# The sedsystem argument translates -I to -ISystem, which suppresses compiler
# warnings coming from the library headers.
guile2_CFLAGS := $(guile (augmk/pkg-config/cflags "guile-2.0" \#:sedsystem \#t))
guile2_LIBS   := $(guile (augmk/pkg-config/libs   "guile-2.0"))

# Targets to build by default.  Subfolders will register their targets here.
ALL_TARGETS :=

# Targets to clean.
CLEAN_TARGETS :=

# Phony targets that perform installation.
INSTALL_TARGETS :=

# Visit the include, src, and guile subdirs.
$(guile (augmk/enter "include"))
$(guile (augmk/enter "src"))
$(guile (augmk/enter "guile"))

# The "all" target builds everything in the target group ALL_TARGETS.
.DEFAULT_GOAL := all
.PHONY: all
all: $(ALL_TARGETS)

# We use augmk's fancy clean support.
$(guile (augmk/create-clean-target "clean" "$(CLEAN_TARGETS)"))

# The "install" target.  Respects DESTDIR.
.PHONY: install
install: $(INSTALL_TARGETS)

test-uniform-sphere-dist: test-uniform-sphere-dist.cc
	g++ $(CXXFLAGS) $(guile2_CFLAGS) -Iinclude -larmadillo -o $@ $<
