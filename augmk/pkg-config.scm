
(define-module (augmk pkg-config))

(use-modules (ice-9 regex))

(use-modules (augmk base))

(define-public (augmk/pkg-config/call-pkgconfig flagtype libs)
  (let* ((libs-str (string-join libs))
         (cmd-str  (string-join `("pkg-config" ,flagtype ,libs-str)))
         (res-str  (augmk/call-cmd cmd-str)))
    (string-delete #\newline res-str)))

;; Retrieve compilation flags for LIBNAMES.
;;
;; Parameters:
;;
;;   * LIBNAMES (list of string): The names of libraries to look up (as
;;     understood by pkg-config).
;;
;;   * SEDSYSTEM (keyword boolean): If not #f, then "-I" entries in the
;;     returned CFLAGS will be translated to "-isystem" entries.  This
;;     suppresses warnings that arise within the headers for this library.
;;     Particularly useful when you want to use -Werror, but a library you use
;;     emits compilation warnings (Looking at you, Eigen).
;;
;; WARNING: The sedsystem option is pretty stupid.  It uses a blind
;; regexp-replace on the returned CFLAGS, which could lead to problems if you
;; have a quoted path that contains a space.  Of course, GNU Make doesn't handle
;; paths with spaces well either.
(define* (augmk/pkg-config/cflags libnames #:key sedsystem)
  (set! libnames (augmk/group-norm libnames))
  (let ((flags (augmk/pkg-config/call-pkgconfig "--cflags" libnames)))
    (when sedsystem
          (set! flags (regexp-substitute/global #f "(^|[[:space:]]+)-I" flags 'pre 1 "-isystem" 'post)))
    flags))

(export augmk/pkg-config/cflags)
  
;; Retrieve linker flags for LIBNAMES.
(define-public (augmk/pkg-config/libs libnames)
  (set! libnames (augmk/group-norm libnames))
  (augmk/pkg-config/call-pkgconfig "--libs" libnames))
