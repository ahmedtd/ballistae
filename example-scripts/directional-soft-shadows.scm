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
(define output    (option-ref options 'output "bunny-directional-soft-shadows.pfm"))

(define scene (bsta/scene/make))

(define white-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.8 0.8 0.8)))))

(define blue-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.2 0.2 0.9)))))

(define green-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.2 0.9 0.2)))))

(define red-mtlmap
  (bsta/mtlmap1/make scene 'constant `((spectrum . ,(bsta/dsig/rgb-to-spectral 0.9 0.2 0.2)))))

(define checkerboard-small
  (bsta/mtlmap1/make scene 'checkerboard `((period . 0.01)
                                           (volumetric . #t))))

(define white-blue-checkerboard
  (bsta/mtlmap1/make scene 'lerp `((t . ,checkerboard-small)
                                   (a . ,white-mtlmap)
                                   (b . ,blue-mtlmap))))

(define white-green-checkerboard
  (bsta/mtlmap1/make
   scene
   'lerp
   `((t . ,(bsta/mtlmap1/make scene 'checkerboard `((period . 1)
                                                    (volumetric . #f))))
     (a . ,white-mtlmap)
     (b . ,green-mtlmap))))

(define red-marble-mtlmap
  (bsta/mtlmap1/make
   scene
   'level
   `((t . ,(bsta/mtlmap1/make scene 'perlinval `((volumetric . #t)
                                                 (period . 1.28))))
     (t-lo . -0.8)
     (t-hi .  0.8)
     (a . ,red-mtlmap)
     (b . ,white-mtlmap))))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "plane" `())
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,white-green-checkerboard)))
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3 0 0 1)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 1 0 0))
  (bsta/aff-t/translation (frst/dvec3 0 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "infinity" `())
 (bsta/matr/make scene "directional_emitter" `())
 (bsta/aff-t/compose
  (bsta/aff-t/basis-mapping (frst/dvec3 1 0 0)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 0 0 1))
  (bsta/aff-t/translation (frst/dvec3 -10 0 0))))


(define bunny-geom
  (bsta/geom/make scene "surface_mesh" `((file . "bunny.obj")
                                         (swapyz . #t))))

(bsta/scene/add-element
 scene
 bunny-geom
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,white-blue-checkerboard)))
 (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) 1.0)
  (bsta/aff-t/scaling 30)
  (bsta/aff-t/translation (frst/dvec3 -2 0 0))))

(bsta/scene/add-element
 scene
 bunny-geom
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,red-marble-mtlmap)))
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
