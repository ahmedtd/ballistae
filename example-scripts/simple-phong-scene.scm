(use-modules (armadillo))
(use-modules (ballistae))

;; A material for the sky.
(define sky-matr
  (bsta/matr/make
   "phong"
   `((emission-wavelength-mean   . 500)
     (emission-wavelength-stddev . 1000)
     (emission-peak-power        . 100)
     (prop-wavelength-mean . 500)
     (prop-wavelength-stddev . 100)
     (prop-peak-k . 0.0))))

;; A material that will appear matte red.
(define red-matr
  (bsta/matr/make "phong"
                  `((emission-peak-power . 0)
                    (prop-wavelength-mean . 710)
                    (prop-wavelength-stddev . 100)
                    (prop-peak-k . 1.0))))

(define scene (bsta/scene/make))

;; Add a ground plane.
(bsta/scene/add scene
                (bsta/geom/make "plane" `())
                red-matr
                (bsta/aff-t/basis-mapping
                 (arma/dvec '(0 0 1))
                 (arma/dvec '(1 0 0))
                 (arma/dvec '(0 1 0))))

;; Add a mesh define from an obj file.
(bsta/scene/add scene
                (bsta/geom/make "surface_mesh" `((file . "dodecahedron.obj")))
                red-matr
                (bsta/aff-t/*
                 (bsta/aff-t/scaling 3)
                 (bsta/aff-t/translation (arma/dvec '(0 0 3)))))

;; Add the sky, without a transform.
(bsta/scene/add scene
                (bsta/geom/make "infty"'())
                sky-matr)

(define cam
  (bsta/cam/make
   "pinhole"
   `((center . ,(arma/dvec '(-8 -8 6)))
     (eye . ,(arma/dvec '(1 1 -0.75)))
     (aperture-vec . ,(arma/dvec '(0.04 0.018 0.012))))))

(bsta/scene/crush scene)

(define (linspace src lim n)
  (let* ((result-list `(,src))
         (step (/ (- lim src) n))
         (cur-val (+ src step)))
    (while (< cur-val lim)
           (set! result-list (append! result-list `(,cur-val)))
           (set! cur-val (+ cur-val step)))
    result-list))

;; Visible wavelengths run from about 390 nm to 835 nm
(bsta/scene/render
 scene cam
 "simple-phong-scene.pfm"
 864 1296
 '(1 16 16)
 ;;(linspace 390 835 5)
 '(700)
 0
 )
