(define-module (ballistae))

(use-modules (ice-9 optargs))
(use-modules (frustum0))

;; Most of our functions are defined in the adapter library.
(load-extension "libguile_ballistae" "libguile_ballistae_init")

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions for asset-mapping.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Assets must be found on disk -- these functions locate assets relative to the
;; currently-executing script file.

(define-public bsta/asset-dir
  (lambda () (dirname (list-ref (command-line) 0))))

(define-public (bsta/asset-realpath relpath)

  (let ((realpath (string-append (bsta/asset-dir)
                                 file-name-separator-string
                                 relpath)))

    (display "Binding asset: ")
    (display relpath)
    (display " -> ")
    (display realpath)
    (newline)

    realpath))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions for generating sampling distributions.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public (bsta/linspace src lim n)
  (let* ((result-list `(,src))
         (step (/ (- lim src) n))
         (cur-val (+ src step)))
    (while (< cur-val lim)
           (set! result-list (append! result-list `(,cur-val)))
           (set! cur-val (+ cur-val step)))
    result-list))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions for dense signals.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public (bsta/dsig/from-list src-val lim-val val-list)
  (bsta/backend/signal/from-list src-val lim-val val-list))

(define-public (bsta/dsig/from-fn src-val lim-val n fn)
  (bsta/backend/signal/from-fn src-val lim-val n fn))

(define-public (bsta/dsig/pulse pulse-src pulse-lim pulse-power)
  (bsta/backend/signal/pulse pulse-src pulse-lim pulse-power))

(define-public (bsta/dsig/sunlight intensity)
  (bsta/backend/signal/sunlight intensity))

(define-public (bsta/dsig/cie-a)
  (bsta/backend/signal/cie-a))

(define-public (bsta/dsig/cie-d65)
  (bsta/backend/signal/cie-d65))

(define-public (bsta/dsig/rgb-to-spectral red green blue)
  (bsta/backend/signal/rgb-to-spectral red green blue))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions for dealing with geometry.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public (bsta/geom/make scene type config-alist)
  (case type
    (else (let* ((soname (string-append "ballistae_geometry_" type))
                 (sohndl (dynamic-link soname))
                 (create-fn (dynamic-pointer "guile_ballistae_geometry" sohndl)))
            (bsta/backend/geom/plugin scene create-fn config-alist)))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Material maps: Bindings from 2D/3D material coordinates to data.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public (bsta/mtlmap1/make scene type config)
  "Create a material map that turns material coordinates into scalar values.

Arguments:

  * SCENE: Ballistae scene in which to define the material map.

  * TYPE: A symbol corresponding to a built-in material map type, or a string
      that names a plugin material map type.

  * CONFIG: A configuration alist for the material map."
  (case type
    ((constant) (bsta/backend/mtlmap1/constant scene config))
    ((lerp) (bsta/backend/mtlmap1/lerp scene config))
    ((level) (bsta/backend/mtlmap1/level scene config))
    ((checkerboard) (bsta/backend/mtlmap1/checkerboard scene config))
    ((bullseye) (bsta/backend/mtlmap1/bullseye scene config))
    ((perlinval) (bsta/backend/mtlmap1/perlinval scene config))
    (else (let* ((soname (string-append "ballistae_mtlmap1_" type))
                (sohndl (dynamic-link soname))
                (create-fn (dynamic-pointer "guile_ballistae_mtlmap1" sohndl)))
           (bsta/backend/mtlmap1/plugin scene create-fn config)))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions for materials.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public (bsta/matr/make scene type config-alist)
  (case type
    (else (let* ((soname (string-append "ballistae_material_" type))
                 (sohndl (dynamic-link soname))
                 (create-fn (dynamic-pointer "guile_ballistae_material" sohndl)))
            (bsta/backend/matr/plugin scene create-fn config-alist)))))

(define-public (bsta/matr/update plugname material config)
  (let* ((soname (string-append "ballistae_material_" plugname))
         (sohndl (dynamic-link soname))
         (update-fn (dynamic-pointer "guile_ballistae_update_material" sohndl)))
    (bsta/backend/matr/update update-fn material config)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions for illuminators
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public (bsta/illum/make scene type config)
  (case type
    ((directional) (bsta/backend/illum/directional scene config))
    ((point) (bsta/backend/illum/point scene config))
    (else (let* ((soname (string-append "ballistae_illuminator_" type))
                 (sohndl (dynamic-link soname))
                 (create-fn (dynamic-pointer "guile_ballistae_illuminator" sohndl)))
            (bsta/backend/illum/make scene create-fn config)))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions dealing with cameras.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public (bsta/cam/make plug-soname config-alist)
  (bsta/backend/cam/make plug-soname config-alist))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions for composing and rendering scenes.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public bsta/scene/make
  (lambda ()
    "Create a new scene."
    (bsta/backend/scene/make)))

(define*-public (bsta/scene/add-element scene
                                        geometry
                                        material
                                        #:optional transform)
  "Add GEOMETRY to SCENE.

It will be rendered with the specified MATERIAL, and TRANSFORM, if
present, specifies the affine mapping from world space to model space.
"
  ;; If the user didn't specify a transform, use identity.
  (unless transform (set! transform (bsta/aff-t/identity)))
  (bsta/backend/scene/add-element scene geometry material transform))

(define*-public (bsta/scene/set-element-transform scene index tform)
  (bsta/backend/scene/set-element-transform scene index tform))

(define*-public (bsta/scene/crush scene #:optional time)
  "Make SCENE ready for rendering."
  (unless time (set! time 0))
  (bsta/backend/scene/crush scene time))

(define*-public (bsta/scene/render scene
                                   camera
                                   output-file
                                   rows
                                   cols
                                   opts)
  "Render SCENE using CAMERA."
  (bsta/backend/scene/render scene
                             camera
                             output-file
                             rows
                             cols
                             opts))
