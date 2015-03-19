(use-modules (armadillo))
(use-modules (ballistae))

(define cam
  (ballistae/camera
   "pinhole"
   `((center . ,(arma/list->b64col '(-10 0 3)))
     (aperture-vec . ,(arma/list->b64col '(1 1.6 .9))))))

(define light-matr
  (ballistae/matr/make "phong"
                       `((emission-wavelength-mean   . 500)
                         (emission-wavelength-stddev . 1000)
                         (emission-peak-power        . 100)
                         (prop-wavelength-mean . 500)
                         (prop-wavelength-stddev . 100)
                         (prop-peak-k . 0.0))))

(define red-matr
  (ballistae/matr/make "phong"
                       `((emission-peak-power . 0)
                         (prop-wavelength-mean . 710)
                         (prop-wavelength-stddev . 100)
                         (prop-peak-k . 1.0))))

(define my-geom
  (ballistae/geom/make
   "sphere"
   `((center . ,(arma/list->b64col '(10 1 1)))
     (radius . 4))))

(define ground-plane
  (ballistae/geom/make "plane"
                       `((center . ,(arma/list->b64col '(0 0 -1)))
                         (normal . ,(arma/list->b64col '(0 0 1))))))

(define back-plane
  (ballistae/geom/make "plane"
                       `((center . ,(arma/list->b64col '(20 0 0)))
                         (normal . ,(arma/list->b64col '(0 0 1))))))

(define light-sphere
  (ballistae/geom/make
   "sphere"
   `((center . ,(arma/list->b64col '(10 -10 10)))
     (radius . 4))))

(define my-scene
  (ballistae/scene/crush
   `((,my-geom      . ,red-matr)
     (,ground-plane . ,red-matr)
     (,light-sphere . ,light-matr))))

(ballistae/render-scene
 cam my-scene
 "simple-phong-scene.pfm"
 450 800
 0
 '(1 16 16 16))
