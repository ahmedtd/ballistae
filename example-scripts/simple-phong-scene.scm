(use-modules (armadillo))
(use-modules (ballistae))

(define cam
  (ballistae/camera
   "pinhole"
   `((center . ,(arma/list->b64col '(-10 0 3)))
     (aperture-vec . ,(arma/list->b64col '(1 1.6 .9))))))

(define pointlight-a (arma/list->b64col '(5 5 10)))
(define pointlight-b (arma/list->b64col '(0 0  2)))

(define dirlight-a (arma/list->b64col '(0 10 -10)))

(define spotlight-a
  `((pos . ,(arma/list->b64col '(20 -10 10)))
    (dir . ,(arma/list->b64col '(-1   1 -1)))
    (cutoff-angle . 0.8)))

(define spotlight-b
  `((pos . ,(arma/list->b64col '(5 5 20)))
    (dir . ,(arma/list->b64col '(0 0 -1)))
    (cutoff-angle . 0.4)))

(define my-matr
  (ballistae/matr/make
   "phong"
   `((k_a   . 0.5)
     (k_d   . 2.0)
     (k_s   . 1.0)
     (alpha . 2)
     (color_a . (1 0 0))
     (color_d . (1 0 0))
     (color_s . (1 1 1))
     ;;(point-lights . (,pointlight-a ,pointlight-b))
     (dir-lights   . (,dirlight-a))
     ;;(spot-lights . (,spotlight-a ,spotlight-b))
     )))

(define my-geom
  (ballistae/geom/make
   "sphere"
   `((center . ,(arma/list->b64col '(10 1 1)))
     (radius . 4))))

(define ground-plane
  (ballistae/geom/make "plane"
                       `((center . ,(arma/list->b64col '(0 0 -1)))
                         (normal . ,(arma/list->b64col '(0 0 1))))))

(define cyl
  (ballistae/geom/make "cylinder"
                       `((center . ,(arma/list->b64col '(5 5 5)))
                         (axis   . ,(arma/list->b64col '(1 1 1))))))

(define my-scene
  (ballistae/scene/crush
   `((,my-geom      . ,my-matr)
     (,ground-plane . ,my-matr)
     (,cyl          . ,my-matr))))

(ballistae/render-scene cam my-scene "simple-phong-scene.jpeg" 800 450 2)
