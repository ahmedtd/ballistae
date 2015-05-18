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
(define output    (option-ref options 'output "reflection-demo.pfm"))

(define scene (bsta/scene/make))

(define white-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.8 0.8 0.8)))))

(define blue-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.2 0.2 0.9)))))

(define green-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.2 0.9 0.2)))))

(define red-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.9 0.2 0.2)))))

(define perlin-blue
  (bsta/mtlmap1/make
   scene
   'level
   `((t . ,(bsta/mtlmap1/make scene 'perlinval `((volumetric . #t)
                                                 (period . 1))))
     (t-lo . -0.8)
     (t-hi .  0.8)
     (a . ,blue-mtlmap)
     (b . ,white-mtlmap))))

(define checkerboard-green
  (bsta/mtlmap1/make
   scene
   'lerp
   `((t . ,(bsta/mtlmap1/make scene 'checkerboard `((volumetric . #f)
                                                    (period . 1))))
     (a . ,green-mtlmap)
     (b . ,white-mtlmap))))

(define perlin-red
  (bsta/mtlmap1/make
   scene
   'level
   `((t . ,(bsta/mtlmap1/make scene 'perlinval `((volumetric . #t)
                                                 (period . 128))))
     (t-lo . -0.8)
     (t-hi .  0.8)
     (a . ,red-mtlmap)
     (b . ,white-mtlmap))))

(define simple-reflective-mtlmap
  (bsta/mtlmap1/make
   scene
   'constant
   `((spectrum . ,(bsta/dsig/pulse 390 835 1.0)))))

(define (iridescent-refl-fn-1 wl)
  (let* ((x-dist (- 835 390))
         (t (/ (- wl 390) x-dist)))
    (+ (* (- 1 t) 0.7) (* t 0.1))))

(define (iridescent-refl-fn-2 wl)
  (let* ((x-dist (- 835 390))
         (t (/ (- wl 390) x-dist)))
    (+ (* (- 1 t) 0.1) (* t 0.9))))

;; A material map that lerps between two different dispersive reflectance maps
;; according to a Perlin control map.  Hopefully, it will look iridescent or
;; opalescent, and not like crap.
;;
;; Perlin period is sized for sphere, not bunny.
(define iridescent-mtlmap
  (bsta/mtlmap1/make
   scene
   'lerp
   `((t . ,(bsta/mtlmap1/make scene 'perlinval `((volumetric . #t)
                                                 (period . 64))))
     (t-lo . -0.6)
     (t-hi . 0.6)
     (a . ,(bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/from-fn 390 835 89 iridescent-refl-fn-1)))))
     (b . ,(bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/from-fn 390 835 89 iridescent-refl-fn-2))))))))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "plane" `())
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,checkerboard-green)))
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

(define sphere-geom
  (bsta/geom/make scene "sphere" `()))

(bsta/scene/add-element
 scene
 sphere-geom
 (bsta/matr/make scene "pc_smooth" `((reflectance . ,simple-reflective-mtlmap)))
  (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 2)
  (bsta/aff-t/translation (frst/dvec3 -3 3 2))))

(bsta/scene/add-element
 scene
 sphere-geom
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,perlin-red)))
  (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 1)
  (bsta/aff-t/translation (frst/dvec3 -3 3 6))))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "surface_mesh" `((file . "bunny.obj") (swapyz . #t)))
 (bsta/matr/make scene "pc_smooth" `((reflectance . ,simple-reflective-mtlmap)))
  (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 30)
  (bsta/aff-t/translation (frst/dvec3 0 0 0))))

(bsta/scene/add-element
 scene
 sphere-geom
 (bsta/matr/make scene "pc_smooth" `((reflectance . ,iridescent-mtlmap)))
  (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 2)
  (bsta/aff-t/translation (frst/dvec3 3 -3 2))))

(bsta/scene/add-element
 scene
 sphere-geom
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,perlin-red)))
  (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 1)
  (bsta/aff-t/translation (frst/dvec3 3 -3 6))))

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
