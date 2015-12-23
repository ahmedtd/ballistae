CC  := gcc
CXX := g++

# We use C++14, with GNU extensions, across the board.
override CXXFLAGS += -std=c++14 -Wall -Werror -Wl,--no-undefined

# Find frustum
frustum0_CFLAGS :=
frustum0_LIBS :=

# Targets to build by default.  Subfolders will register their targets here.
ALL_TARGETS :=

# Targets to clean.
CLEAN_TARGETS :=

# Phony targets that perform installation.
INSTALL_TARGETS :=

# Visit the src subdir.
$(guile (augmk/enter "src"))
$(guile (augmk/enter "examples"))

# Install include files.  Relpath is empty, since install-hdr-phony places the
# files in $(DESTDIR)/include by default, which is where we want them.
INSTALL_TARGETS += $(guile (augmk/install-hdr-phony "hdr_install"         \
													""                    \
													"$(augmk_d)/include/"))

# The "all" target builds everything in the target group ALL_TARGETS.
.DEFAULT_GOAL := all
.PHONY: all
all: $(ALL_TARGETS)

# We use augmk's fancy clean support.
$(guile (augmk/create-clean-target "clean" "$(CLEAN_TARGETS)"))

# The "install" target.  Respects DESTDIR.
.PHONY: install
install: $(INSTALL_TARGETS)
