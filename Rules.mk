CC  := gcc
CXX := g++

# We use C++14, with GNU extensions, across the board.
override CXXFLAGS += -std=gnu++1y -Wall -Werror -Wl,--no-undefined -fno-signed-zeros -fno-trapping-math -fno-rounding-math -fassociative-math

# Find frustum
frustum0_CFLAGS :=
frustum0_LIBS :=

libguile_frustum0_CFLAGS :=
libguile_frustum0_LIBS := -lguile_frustum0

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
$(guile (augmk/enter "src"))

# Install include files.  Relpath is empty, since install-hdr-phony places the
# files in $(DESTDIR)/include by default, which is where we want them.
INSTALL_TARGETS += $(guile (augmk/install-hdr-phony "hdr_install"         \
													""                    \
													"$(augmk_d)/include/"))

# Install guile source files.
#
# Note that augmk uses rsync to perform installs, so the trailing slash in
# $(augmk_d)/guile/ IS signficant.  It means that the contents of the folder are
# copied to the destination, not the folder itself.
INSTALL_TARGETS += $(guile (augmk/install-phony "guile_src_install" \
												"/share/guile/"     \
												"a=r,u=rw"          \
												"$(augmk_d)/guile/"))

# The "all" target builds everything in the target group ALL_TARGETS.
.DEFAULT_GOAL := all
.PHONY: all
all: $(ALL_TARGETS)

# We use augmk's fancy clean support.
$(guile (augmk/create-clean-target "clean" "$(CLEAN_TARGETS)"))

# The "install" target.  Respects DESTDIR.
.PHONY: install
install: $(INSTALL_TARGETS)
