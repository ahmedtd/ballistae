(define-module (ballistae))

(use-modules (ice-9 optargs))
(use-modules (armadillo))

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
;; Functions dealing with affine transform subsmobs.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public bsta/aff-t/identity
  (lambda ()
    "The identity transform."
    (bsta/backend/aff-t/identity)))

(define-public (bsta/aff-t/translation offset)
  "A transform that translates by OFFSET."
  (bsta/backend/aff-t/translation offset))

(define-public (bsta/aff-t/scaling scalar)
  "A transform that uniformly scales by SCALAR."
  (bsta/backend/aff-t/scaling scalar))

(define-public (bsta/aff-t/rotation axis angle)
  "A transform that rotates around AXIS by ANGLE (radians).

AXIS does not need to be normalized."
  (bsta/backend/aff-t/rotation axis angle))

(define-public (bsta/aff-t/basis-mapping e0 e1 e2)
  "A transform that maps the current principle axes to E0,E1,E2."
  (bsta/backend/aff-t/basis-mapping e0 e1 e2))

(define*-public (bsta/aff-t/* #:rest tform-list)
  "Compose TFORM-LIST to a single transform, from left to right."
  (apply bsta/backend/aff-t/compose tform-list))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions for dealing with geometry.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public (bsta/geom/make plug-soname config-alist)
  (bsta/backend/geom/make plug-soname config-alist))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions for dealing with materials.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public (bsta/matr/make plug-soname config-alist)
  (bsta/backend/matr/make plug-soname config-alist))

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

(define*-public (bsta/scene/add scene geometry material #:optional transform)
  "Add GEOMETRY to SCENE.

It will be rendered with the specified MATERIAL, and TRANSFORM, if
present, specifies the affine mapping from world space to model space.
"
  ;; If the user didn't specify a transform, use identity.
  (unless transform (set! transform (bsta/aff-t/identity)))
  (bsta/backend/scene/add scene geometry material transform))

(define*-public (bsta/scene/crush scene)
  "Make SCENE ready for rendering."
  (bsta/backend/scene/crush scene))

(define*-public (bsta/scene/render scene
                                   camera
                                   output-file
                                   rows
                                   cols
                                   recursion-profile
                                   lambda-profile
                                   ss-factor)
  "Render SCENE using CAMERA."
  (bsta/backend/scene/render scene
                             camera
                             output-file
                             rows
                             cols
                             ss-factor
                             recursion-profile
                             lambda-profile))
