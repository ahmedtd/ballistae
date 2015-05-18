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
(define output-folder (option-ref options 'output "index-of-refraction-animation"))

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
 (bsta/matr/make "directional_emitter" `())
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

(define (dispersive-n wl t)
  (define wl-range (- 835 390))
  (define s (/ (- wl 390) wl-range))
  (define nlo (+ 1 (/ t 2)))
  (define nhi (+ 1 (/ t 4)))
  (lerp nlo nhi s))

(bsta/scene/add-element
 scene
 (bsta/geom/make "sphere" `())
 (bsta/matr/make "nonconductive_smooth" `((n-interior . ,(bsta/dsig/from-fn 390 835 89 (lambda (wl) (dispersive-n wl 3))))))
 (bsta/aff-t/compose
  (bsta/aff-t/scaling 2)
  (bsta/aff-t/translation (frst/dvec3 -5 2.5 2))))

(define sphere-index
  (bsta/scene/add-element
   scene
   (bsta/geom/make "sphere" `())
   (bsta/matr/make "nonconductive_smooth" `((n-interior . ,(bsta/dsig/from-fn 390 835 89 (lambda (wl) (dispersive-n wl 0))))))
   (bsta/aff-t/compose
    (bsta/aff-t/scaling 2)
    (bsta/aff-t/translation (frst/dvec3 -5 -2.5 2)))))

(bsta/scene/add-element
 scene
 (bsta/geom/make "surface_mesh" `((file . "bunny-lo.obj") (swapyz . #t)))
 (bsta/matr/make "mc_lambert" `())
 (bsta/aff-t/compose
  (bsta/aff-t/scaling 5)
  (bsta/aff-t/translation (frst/dvec3 -5 -2.5 2))))

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

   (define sphere-material (bsta/scene/get-element-material scene sphere-index))
   (bsta/matr/update
    "nonconductive_smooth"
    sphere-material
    `((n-interior . ,(bsta/dsig/from-fn 390 835 89 (lambda (wl) (dispersive-n wl t))))))

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
