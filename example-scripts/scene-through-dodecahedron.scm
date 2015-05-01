(use-modules (ice-9 getopt-long))
(use-modules (ice-9 format))

(use-modules (frustum0))
(use-modules (ballistae))

(define option-spec
  '((frame-num  (value #t))
    (frame-rate (value #t))
    (gridsize   (value #t))
    (nlambdas   (value #t))
    (depthlim   (value #t))))

(define options (getopt-long (command-line) option-spec))

(define frame-num  (string->number (option-ref options 'frame-num  "0")))
(define frame-rate (string->number (option-ref options 'frame-rate "24")))
(define gridsize  (string->number (option-ref options 'gridsize  "4")))
(define nlambdas  (string->number (option-ref options 'nlambdas  "16")))
(define depthlim  (string->number (option-ref options 'depthlim  "16")))

;; Turn the frame specification into a usable time.
(define t (/ frame-num frame-rate))

(define scene (bsta/scene/make))

;; Add a white ground plane
(bsta/scene/add-element
 scene
 (bsta/geom/make "plane" `())
 (bsta/matr/make "phong" `((ambient . ,(bsta/dsig/pulse 390 835 20))
                           (diffuse . ,(bsta/dsig/pulse 390 835 1))))
 (bsta/aff-t/basis-mapping (frst/dvec3 0 0 1)
                           (frst/dvec3 0 1 0)
                           (frst/dvec3 1 0 0)))

;; Add a diamond dodecahedron.
;;
;; The swapyz setting corrects for maya-style axes.
(bsta/scene/add-element
 scene
 (bsta/geom/make "surface_mesh" `((file . "dodecahedron.obj") (swapyz . #t)))
 (bsta/matr/make "nonconductive_smooth"
                  `((n-interior . 2.41)
                    (n-exterior . 1)))
 (bsta/aff-t/compose
  (bsta/aff-t/rotation (frst/dvec3 0 0 1) (* t (* 0.3 3.14159)))
  (bsta/aff-t/scaling 5)
  (bsta/aff-t/translation (frst/dvec3 0 0 7))
  ))

;; Add a blue sphere
(bsta/scene/add-element
 scene
 (bsta/geom/make "sphere" `())
 (bsta/matr/make "phong" `((ambient . ,(bsta/dsig/rgb-to-spectral 1 1 9))
                           (diffuse . ,(bsta/dsig/rgb-to-spectral 0.1 0.1 0.9))))
 (bsta/aff-t/compose
  (bsta/aff-t/scaling 20)
  (bsta/aff-t/translation (frst/dvec3 30 20 0))))

;; Add a red sphere
(bsta/scene/add-element
 scene
 (bsta/geom/make "sphere" `())
 (bsta/matr/make "phong" `((ambient . ,(bsta/dsig/rgb-to-spectral 9 1 1))
                           (diffuse . ,(bsta/dsig/rgb-to-spectral 0.9 0.1 0.1))))
 (bsta/aff-t/compose
  (bsta/aff-t/scaling 2)
  (bsta/aff-t/translation (frst/dvec3 10 14 10))))

;; Add a single directional light.
(bsta/scene/add-illuminator
 scene
 (bsta/illum/make "dir" `((spectrum  . ,(bsta/dsig/pulse 390 835 100))
                          (direction . ,(frst/dvec3 1 0 -1)))))

(define cam (bsta/cam/make "pinhole" `((center . ,(frst/dvec3 -20 -20    10))
                                       (eye    . ,(frst/dvec3   1   1    -0.2))
                                       (aperture-vec . ,(frst/dvec3 0.05 0.018 0.012)))))

(bsta/scene/crush scene)

(define output-name (format #f "scene-through-dodecahedron-~d.pfm" frame-num))

(bsta/scene/render scene cam
                   output-name
                   864 1296
                   `((gridsize . ,gridsize)
                     (nlambdas . ,nlambdas)
                     (depthlim . ,depthlim)))
