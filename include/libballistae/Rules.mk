# Install header files into /usr/include/libballistae (or analagous).
$(augmk_d)_headers = $(wildcard $(augmk_d)/*.hh)
INSTALL_TARGETS += $(guile (augmk/install-hdr-phony "$(augmk_d)_install" "libballistae" "$($(augmk_d)_headers)"))
