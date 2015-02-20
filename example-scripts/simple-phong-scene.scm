(use-modules (armadillo))
(use-modules (ballistae))

(define cam
  (ballistae/camera
   "pinhole"
   `((center . ,(arma/list->b64col '(-10 0 3))))))

(define infty-matr
  (ballistae/matr/make
   "phong"
   `((color_a . (0 0 0)))))

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
     (lights . (,(arma/list->b64col '(0 10 10)))))))

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
   infty-matr
   `((,my-geom      . ,my-matr)
     (,ground-plane . ,my-matr)
     (,cyl          . ,my-matr))))

(ballistae/render-scene cam my-scene "simple-phong-scene.jpeg" 512 512 2)
