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
(define output    (option-ref options 'output "mc-simple-texmap.pfm"))

(define scene (bsta/scene/make))

(define white-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.8 0.8 0.8)))))

(define blue-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.2 0.2 0.8)))))

(define green-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.2 0.8 0.2)))))

(define red-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.8 0.2 0.2)))))

(define bullseye-2d-blue
  (bsta/mtlmap1/make
   scene
   'level
   `((t . ,(bsta/mtlmap1/make scene 'bullseye `((volumetric . #f)
                                                 (period . 0.5))))
     (a . ,blue-mtlmap)
     (b . ,white-mtlmap))))

(define bullseye-2d-green
  (bsta/mtlmap1/make
   scene
   'level
   `((t . ,(bsta/mtlmap1/make scene 'bullseye `((volumetric . #f)
                                                 (period . 2))))
     (a . ,green-mtlmap)
     (b . ,white-mtlmap))))

(define perlin-2d-red
  (bsta/mtlmap1/make
   scene
   'level
   `((t . ,(bsta/mtlmap1/make scene 'perlinval `((volumetric . #f)
                                                 (period . 128))))
     (t-lo . -0.8)
     (t-hi .  0.8)
     (a . ,red-mtlmap)
     (b . ,white-mtlmap))))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "plane" `())
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,bullseye-2d-green)))
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3 0 0 1)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 1 0 0))
  (bsta/aff-t/translation (frst/dvec3 0 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "infinity" `())
 (bsta/matr/make
  scene
  "directional_emitter"
  `((spectrum . ,(bsta/dsig/cie-d65))
    (dir . ,(frst/dvec3 -1 -1 -0.5))
    (cutoff . 0.95)
    (lo-level . 0.05)
    (hi-level . 1.0)))
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3 1 0 0)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 0 0 1))
  (bsta/aff-t/translation (frst/dvec3 -10 0 0))))

(define sphere-geom
  (bsta/geom/make scene "sphere" `()))

(bsta/scene/add-element
 scene
 sphere-geom
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,bullseye-2d-blue)))
 (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 2)
  (bsta/aff-t/translation (frst/dvec3 -3 1 2))))

(bsta/scene/add-element
 scene
 sphere-geom
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,perlin-2d-red)))
 (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 2)
  (bsta/aff-t/translation (frst/dvec3 3 -3 2))))

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
