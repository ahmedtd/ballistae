#
# Build system for the ballistae ray-tracer.
#

# We don't use any GNU Make built-in rules.
.SUFFIXES:

# We use augmk, which requires guile-2 support in Make.
ifneq (guile,$(findstring guile,$(.FEATURES)))
	$(error This Makefile uses augmk, which requires GNU Guile support.)
endif

# Tell Guile where to find augmk.
$(guile (add-to-load-path "./"))

# Load the components of augmk that we use.
$(guile (use-modules (augmk base)))
$(guile (use-modules (augmk gcc)))
$(guile (use-modules (augmk pkg-config)))

# Load the top-level Rules.mk
$(guile (augmk/enter "."))
