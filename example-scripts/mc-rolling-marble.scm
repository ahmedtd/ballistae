(use-modules (ice-9 getopt-long))
(use-modules (ice-9 format))

(use-modules (frustum0))
(use-modules (ballistae))

(define option-spec
  '((frame-src     (value #t))
    (frame-lim     (value #t))
    (frame-rate    (value #t))
    (gridsize      (value #t))
    (nlambdas      (value #t))
    (depthlim      (value #t))
    (output-folder (value #t))))

(define options (getopt-long (command-line) option-spec))

(define frame-src  (string->number (option-ref options 'frame-src  "0")))
(define frame-lim  (string->number (option-ref options 'frame-lim  "48")))
(define frame-rate (string->number (option-ref options 'frame-rate "24")))
(define gridsize  (string->number (option-ref options 'gridsize  "4")))
(define nlambdas  (string->number (option-ref options 'nlambdas  "16")))
(define depthlim  (string->number (option-ref options 'depthlim  "16")))
(define output-folder (option-ref options 'output "mc-rolling-marble"))

(when (file-exists? output-folder)
      (error "Output folder ~a already exists." output-folder))
(mkdir output-folder)

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
 (bsta/geom/make "plane" `())
 (bsta/matr/make "mc_lambert" `())
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3  0 0 1)
                            (frst/dvec3  0 1 0)
                            (frst/dvec3 -1 0 0))
  (bsta/aff-t/translation (frst/dvec3 0 0 20))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "omnidirectional_emitter" `((spectrum . ,(bsta/dsig/cie-d65))))
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3 1 0 0)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 0 0 1))
  (bsta/aff-t/translation (frst/dvec3 -10 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "mc_lambert" `((reflectance . ,(bsta/dsig/rgb-to-spectral 0.1 0.1 0.95))))
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3 -1 0 0)
                            (frst/dvec3  0 1 0)
                            (frst/dvec3  0 0 1))
  (bsta/aff-t/translation (frst/dvec3 10 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "mc_lambert" `((reflectance . ,(bsta/dsig/rgb-to-spectral 0.1 0.95 0.1))))
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3 0 0 1)
                            (frst/dvec3 1 0 0)
                            (frst/dvec3 0 1 0))
  (bsta/aff-t/translation (frst/dvec3 0 -10 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "mc_lambert" `((reflectance . ,(bsta/dsig/rgb-to-spectral 0.95 0.1 0.1))))
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3  0 0 1)
                            (frst/dvec3 -1 0 0)
                            (frst/dvec3  0 1 0))
  (bsta/aff-t/translation (frst/dvec3 0 10 0))))

(define (lerp v1 v2 t)
  (+ (* (- 1 t) v1) (* t v2)))

(define (dispersive-n wl)
  (define wl-range (- 835 390))
  (define s (/ (- wl 390) wl-range))
  (lerp 1.8 1.2 s))

(define (rolling-transform t)
  (bsta/aff-t/compose
   (bsta/aff-t/rotation (frst/dvec3 0 1 0) t)
   (bsta/aff-t/translation (frst/dvec3 (+ -5 (* 2 t)) -2.5 2))))

(define sphere-base-transform
  (bsta/aff-t/scaling 2))

(define sphere-index
  (bsta/scene/add-element
   scene
   (bsta/geom/make "sphere" `())
   (bsta/matr/make "nonconductive_smooth" `((n-interior . ,(bsta/dsig/from-fn 390 835 89 dispersive-n))))
   sphere-base-transform))

(define bubble-0-base-transform
  (bsta/aff-t/compose
   (bsta/aff-t/scaling 0.2)
   (bsta/aff-t/translation (frst/dvec3 1.03 0.21 0.67))))

(define bubble-1-base-transform
  (bsta/aff-t/compose
   (bsta/aff-t/scaling 0.39)
   (bsta/aff-t/translation (frst/dvec3 -0.86 0.39 -0.29))))

(define bubble-2-base-transform
  (bsta/aff-t/compose
   (bsta/aff-t/scaling 0.25)
   (bsta/aff-t/translation (frst/dvec3 -0.22 -0.8 -0.24))))

(define bubble-3-base-transform
  (bsta/aff-t/compose
   (bsta/aff-t/scaling 0.34)
   (bsta/aff-t/translation (frst/dvec3 0.1 -0.67 1.02))))

(define bubble-4-base-transform
  (bsta/aff-t/compose
   (bsta/aff-t/scaling 0.2)
   (bsta/aff-t/translation (frst/dvec3 1.05 -0.31 0.50))))

(define bubble-0-index
  (bsta/scene/add-element
   scene
   (bsta/geom/make "sphere" `())
   (bsta/matr/make "nonconductive_smooth" `((n-exterior . ,(bsta/dsig/from-fn 390 835 89 dispersive-n))))
   bubble-0-base-transform))

(define bubble-1-index
  (bsta/scene/add-element
   scene
   (bsta/geom/make "sphere" `())
   (bsta/matr/make "nonconductive_smooth" `((n-exterior . ,(bsta/dsig/from-fn 390 835 89 dispersive-n))))
   bubble-1-base-transform))

(define bubble-2-index
  (bsta/scene/add-element
   scene
   (bsta/geom/make "sphere" `())
   (bsta/matr/make "nonconductive_smooth" `((n-exterior . ,(bsta/dsig/from-fn 390 835 89 dispersive-n))))
   bubble-2-base-transform))

(define bubble-3-index
  (bsta/scene/add-element
   scene
   (bsta/geom/make "sphere" `())
   (bsta/matr/make "nonconductive_smooth" `((n-exterior . ,(bsta/dsig/from-fn 390 835 89 dispersive-n))))
   bubble-3-base-transform))

(define bubble-4-index
  (bsta/scene/add-element
   scene
   (bsta/geom/make "sphere" `())
   (bsta/matr/make "nonconductive_smooth" `((n-exterior . ,(bsta/dsig/from-fn 390 835 89 dispersive-n))))
   bubble-4-base-transform))

(bsta/scene/add-element
 scene
 (bsta/geom/make "surface_mesh" `((file . "cube.obj") (swapyz . #t)))
 (bsta/matr/make "mc_lambert" `())
 (bsta/aff-t/compose
  (bsta/aff-t/scaling 5)
  (bsta/aff-t/translation (frst/dvec3 -5 2.5 1))))

(define cam-center (frst/dvec3 -5 -9  5))
(define cam-eye (frst/- (frst/dvec3 -2 0 2) cam-center))

(define cam (bsta/cam/make "pinhole" `((center . ,cam-center)
                                       (eye    . ,cam-eye)
                                       (aperture-vec . ,(frst/dvec3 0.02 0.0192 0.0108)))))

(define oldwd (getcwd))

(do ((frame frame-src (1+ frame)))
    ((= frame frame-lim))

  (let ()

   (define ofile (format #f "~d.pfm" frame))
   (define t     (/ frame frame-rate))

   (bsta/scene/set-element-transform
    scene
    sphere-index
    (bsta/aff-t/compose
     sphere-base-transform
     (rolling-transform t)))

   (bsta/scene/set-element-transform
    scene
    bubble-0-index
    (bsta/aff-t/compose
     bubble-0-base-transform
     (rolling-transform t)))

   (bsta/scene/set-element-transform
    scene
    bubble-1-index
    (bsta/aff-t/compose
     bubble-1-base-transform
     (rolling-transform t)))

   (bsta/scene/set-element-transform
    scene
    bubble-2-index
    (bsta/aff-t/compose
     bubble-2-base-transform
     (rolling-transform t)))

   (bsta/scene/set-element-transform
    scene
    bubble-3-index
    (bsta/aff-t/compose
     bubble-3-base-transform
     (rolling-transform t)))

   (bsta/scene/set-element-transform
    scene
    bubble-4-index
    (bsta/aff-t/compose
     bubble-4-base-transform
     (rolling-transform t)))

   ;; Re-crush scene.
   (bsta/scene/crush scene)

   (chdir output-folder)

   ;; Render frame
   (bsta/scene/render scene cam
                      ofile
                      1080 1920
                      `((gridsize . ,gridsize)
                        (nlambdas . ,nlambdas)
                        (depthlim . ,depthlim)))
   (chdir oldwd)))
