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
(define output    (option-ref options 'output "meshes.pfm"))

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

(define perlin-green
  (bsta/mtlmap1/make
   scene
   'level
   `((t . ,(bsta/mtlmap1/make scene 'perlinval `((volumetric . #t)
                                                 (period . 2))))
     (t-lo . -0.8)
     (t-hi .  0.8)
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

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "plane" `())
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,white-mtlmap)))
 (frst/daff3/compose
  (frst/daff3/basis-mapping (frst/dvec3 0 0 1)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 1 0 0))
  (frst/daff3/translation (frst/dvec3 0 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "infinity" `())
 (bsta/matr/make scene "directional_emitter" `())
 (frst/daff3/compose
  (frst/daff3/basis-mapping (frst/dvec3 1 0 0)
                            (frst/dvec3 0 1 0)
                            (frst/dvec3 0 0 1))
  (frst/daff3/translation (frst/dvec3 -10 0 0))))

(define sphere-geom
  (bsta/geom/make scene "sphere" `()))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "surface_mesh" `((file . "tetrahedron.obj")))
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,perlin-blue)))
 (frst/daff3/compose
  (frst/daff3/rotation (frst/dvec3 0 0 1) 1.0)
  (frst/daff3/scaling 2)
  (frst/daff3/translation (frst/dvec3 -2 2 -2))))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "surface_mesh" `((file . "cube.obj")))
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,perlin-red)))
 (frst/daff3/compose
  (frst/daff3/rotation (frst/dvec3 0 0 1) 1.0)
  (frst/daff3/scaling 2)
  (frst/daff3/translation (frst/dvec3 0 0 0))))

(bsta/scene/add-element
 scene
 (bsta/geom/make scene "surface_mesh" `((file . "bunny.obj")
                                        (swapyz . #t)))
 (bsta/matr/make scene "mc_lambert" `((reflectance . ,perlin-green)))
 (frst/daff3/compose
  (frst/daff3/rotation (frst/dvec3 0 0 1) 1.0)
  (frst/daff3/scaling 30)
  (frst/daff3/translation (frst/dvec3 2 -2 0))))

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
