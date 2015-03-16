# Install header files into /usr/include/libguile_ballistae (or analagous).
$(augmk_d)_headers = $(wildcard $(augmk_d)/*.hh)
INSTALL_TARGETS += $(guile (augmk/install-hdr-phony "$(augmk_d)_install" "libguile_ballistae" "$($(augmk_d)_headers)"))
