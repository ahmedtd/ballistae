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
(define output    (option-ref options 'output "mc-cornell-box.pfm"))

(define scene (bsta/scene/make))

(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "mc_lambert" `())
 (frst/daff3/compose
  (frst/daff3/basis-mapping (frst/dvec3 0 0 1)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 1 0 0))
  (frst/daff3/translation (frst/dvec3 0 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "mc_lambert" `())
 (frst/daff3/compose
  (frst/daff3/basis-mapping (frst/dvec3  0 0 1)
                            (frst/dvec3  0 1 0)
                            (frst/dvec3 -1 0 0))
  (frst/daff3/translation (frst/dvec3 0 0 20))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "directional_emitter" `())
 (frst/daff3/compose
  (frst/daff3/basis-mapping (frst/dvec3 1 0 0)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 0 0 1))
  (frst/daff3/translation (frst/dvec3 -10 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "mc_lambert" `())
 (frst/daff3/compose
  (frst/daff3/basis-mapping (frst/dvec3 -1 0 0)
                            (frst/dvec3  0 1 0)
                            (frst/dvec3  0 0 1))
  (frst/daff3/translation (frst/dvec3 10 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "mc_lambert" `((reflectance . ,(bsta/dsig/rgb-to-spectral 0.1 0.95 0.1))))
 (frst/daff3/compose
  (frst/daff3/basis-mapping (frst/dvec3 0 0 1)
                            (frst/dvec3 1 0 0)
                            (frst/dvec3 0 1 0))
  (frst/daff3/translation (frst/dvec3 0 -10 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "mc_lambert" `((reflectance . ,(bsta/dsig/rgb-to-spectral 0.95 0.1 0.1))))
 (frst/daff3/compose
  (frst/daff3/basis-mapping (frst/dvec3  0 0 1)
                            (frst/dvec3 -1 0 0)
                            (frst/dvec3  0 1 0))
  (frst/daff3/translation (frst/dvec3 0 10 0))))

(define (dispersive-n wl)
  (let* ((x-dist (- 835 390))
         (t (/ (- wl 390) x-dist)))
    (+ (* (- 1.7 t) 1) (* t 1.3))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "sphere" `())
 (bsta/matr/make "nonconductive_smooth" `((n-interior . ,(bsta/dsig/from-fn 390 835 89 dispersive-n))))
 (frst/daff3/compose
  (frst/daff3/scaling 2)
  (frst/daff3/translation (frst/dvec3 -5 -2.5 2))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "sphere" `())
 (bsta/matr/make "nonconductive_smooth" `((n-interior . ,(bsta/dsig/from-fn 390 835 89 dispersive-n))))
 (frst/daff3/compose
  (frst/daff3/scaling 2)
  (frst/daff3/translation (frst/dvec3 -5 2.5 2))))

;; (bsta/scene/add-element
;;  scene
;;  (bsta/geom/make "surface_mesh" `((file . "bunny-lo.obj") (swapyz . #t)))
;;  (bsta/matr/make "mc_lambert" `())
;;  (frst/daff3/compose
;;   (frst/daff3/scaling 5)
;;   (frst/daff3/translation (frst/dvec3 -5 -2.5 2))))

(define cam-center (frst/dvec3 -5 -9  5))
(define cam-eye (frst/- (frst/dvec3 -2 0 2) cam-center))

(define cam (bsta/cam/make "pinhole" `((center . ,cam-center)
                                       (eye    . ,cam-eye)
                                       (aperture-vec . ,(frst/dvec3 0.02 0.018 0.012)))))

(bsta/scene/crush scene)

(bsta/scene/render scene cam
                   output
                   864 1296
                   `((gridsize . ,gridsize)
                     (nlambdas . ,nlambdas)
                     (depthlim . ,depthlim)))
