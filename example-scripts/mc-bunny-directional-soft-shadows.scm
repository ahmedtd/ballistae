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
(define output    (option-ref options 'output "mc-bunny-directional-soft-shadows.pfm"))

(define scene (bsta/scene/make))

(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "mc_lambert" `())
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3 0 0 1)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 1 0 0))
  (bsta/aff-t/translation (frst/dvec3 0 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "infinity" `())
 (bsta/matr/make
  "directional_emitter"
  `((spectrum . ,(bsta/dsig/cie-d65))
    (dir . ,(frst/dvec3 -1 -1 -1))
    (cutoff . 0.9)
    (lo-level . 0.2)
    (hi-level . 1.0)))
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3 1 0 0)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 0 0 1))
  (bsta/aff-t/translation (frst/dvec3 -10 0 0))))

(define (dispersive-n wl)
  (let* ((x-dist (- 835 390))
         (t (/ (- wl 390) x-dist)))
    (+ (* (- 1.7 t) 1) (* t 1.3))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "surface_mesh" `((file . "bunny.obj") (swapyz . #t)))
 (bsta/matr/make "mc_lambert" `())
 (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 30)
  (bsta/aff-t/translation (frst/dvec3 -2 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "surface_mesh" `((file . "bunny.obj") (swapyz . #t)))
 (bsta/matr/make "nonconductive_smooth" `((n-interior . ,(bsta/dsig/from-fn 390 835 89 dispersive-n))))
 (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 30)
  (bsta/aff-t/translation (frst/dvec3 2 -2 0))))

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
