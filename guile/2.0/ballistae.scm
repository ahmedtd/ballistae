(define-module (ballistae))

(use-modules (armadillo))

;; Most of our functions are defined in the adapter library.
(load-extension "libguile_ballistae" "libguile_ballistae_init")
