(use-modules (armadillo))
(use-modules (ballistae))

(define scene (bsta/scene/make))

;; Add a white ground plane
(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "phong" `((ambient . ,(bsta/dsig/pulse 390 835 20))
                           (diffuse . ,(bsta/dsig/pulse 390 835 1))))
 (bsta/aff-t/basis-mapping (arma/dvec '(0 0 1))
                           (arma/dvec '(1 0 0))
                           (arma/dvec '(0 1 0))))

;; Add a blue Stanford bunny.
;;
;; The swapyz setting corrects for maya-style axes.
(bsta/scene/add-element
 scene
 (bsta/geom/make "surface_mesh" `((file . "bunny.obj")
                                  (swapyz . #t)))
 (bsta/matr/make "phong" `((ambient . ,(bsta/dsig/rgb-to-spectral 1 1 9))
                           (diffuse . ,(bsta/dsig/rgb-to-spectral 0.1 0.1 0.9))))
 (bsta/aff-t/compose
  (bsta/aff-t/scaling 40)
  (bsta/aff-t/translation (arma/dvec '(0 0 0)))
  ))

;; Add a single directional light.
(bsta/scene/add-illuminator
 scene
 (bsta/illum/make "dir" `((spectrum  . ,(bsta/dsig/pulse 390 835 100))
                          (direction . ,(arma/dvec '(1 0 -1))))))

(define cam (bsta/cam/make "pinhole" `((center . ,(arma/dvec '(-20 -20    12)))
                                       (eye    . ,(arma/dvec '(  1   1 -0.3)))
                                       (aperture-vec . ,(arma/dvec '(0.05 0.018 0.012))))))

(bsta/scene/crush scene)

(bsta/scene/render scene cam
                   "phong-bunny.pfm"  ;; output file
                   864 1296                          ;; rows, columns
                   '(1 1 1 1 1 1)                    ;; depth profile
                   '(390 . 835)                      ;; spectral profile
                   0)                                ;; supersampling factor
