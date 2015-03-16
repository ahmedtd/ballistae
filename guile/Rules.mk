# Install header files into /usr/share/guile/<version> (or analagous).
$(augmk_d)_files = $(wildcard $(augmk_d)/*.scm)
INSTALL_TARGETS += $(guile (augmk/install-phony "$(augmk_d)_install" "/usr/share/guile/$(GUILE_INSTALL_VERSION)" "a=r,u=rw" "$($(augmk_d)_files)"))
