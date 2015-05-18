(use-modules (ice-9 getopt-long))
(use-modules (ice-9 format))

(use-modules (frustum0))
(use-modules (ballistae))

(define option-spec
  '((gridsize   (value #t))
    (nlambdas   (value #t))
    (depthlim   (value #t))
    (output     (value #t))))

(define options (getopt-long (command-line) option-spec))

(define gridsize  (string->number (option-ref options 'gridsize  "4")))
(define nlambdas  (string->number (option-ref options 'nlambdas  "16")))
(define depthlim  (string->number (option-ref options 'depthlim  "16")))
(define output    (option-ref options 'output "volumetric-light.pfm"))

(define scene (bsta/scene/make))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "plane" `())
 (bsta/matr/make scene "mc_lambert" `(reflectance . ,(bsta/dsig/rgb-to-spectral 0.5 0.5 0.5)))
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3 0 0 1)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 1 0 0))
  (bsta/aff-t/translation (frst/dvec3 0 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "infinity" `())
 (bsta/matr/make scene "directional_emitter" `())
 (bsta/aff-t/identity))

(define bunny-geom
  (bsta/geom/make scene "surface_mesh" `((file . "bunny.obj") (swapyz . #t))))

(bsta/scene/add-element
 scene
 bunny-geom
 (bsta/matr/make scene "mc_lambert" `(reflectance . ,(bsta/dsig/rgb-to-spectral 0.5 0.5 0.5)))
 (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 30)
  (bsta/aff-t/translation (frst/dvec3 -0 0 0))))

(define sphere-geom
  (bsta/geom/make scene "sphere" `()))

(bsta/scene/add-element
 scene
 sphere-geom
 (bsta/matr/make scene "emitter" `())
 (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 3)
  (bsta/aff-t/translation (frst/dvec3 3 3 8))))

(define cam-center (frst/dvec3 -5 -11  5))
(define cam-eye (frst/- (frst/dvec3 0 0 2) cam-center))

(define cam (bsta/cam/make "pinhole" `((center . ,cam-center)
                                       (eye    . ,cam-eye)
                                       (aperture-vec . ,(frst/dvec3 0.03 0.018 0.012)))))

(bsta/scene/crush scene)

(bsta/scene/render scene cam
                   output
                   864 1296
                   `((gridsize . ,gridsize)
                     (nlambdas . ,nlambdas)
                     (depthlim . ,depthlim)))
