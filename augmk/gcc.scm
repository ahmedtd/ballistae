
(define-module (augmk gcc))

(use-modules (augmk base))

;; Helper function for invoking gcc in autodep mode.
;;
;; Parameters:
;;
;;   cmd: (string) Executable to invoke.
;;
;;   src: (string) Source file.
;;
;;   obj: (string) Object file.
;;
;;   opts: (string) Options for command.
(define (augmk/gcc/autodep-helper cmd src obj opts)
  (let* ((cmd-str (string-join `(,cmd ,opts "-MT" ,obj "-MM" ,src)))
         (rule     (augmk/call-cmd cmd-str)))
    (augmk/info "Loading autodep info for " obj)
    (augmk/eval rule)))

(define-public (augmk/gcc/autodep-c srcs objs opts)
  (set! srcs (augmk/group-norm srcs))
  (set! objs (augmk/group-norm objs))
  (set! opts (augmk/expand opts))
  (map
   (lambda (src obj)
     (augmk/gcc/autodep-helper (augmk/expand "$(CC)") src obj opts))
   srcs objs))

(define-public (augmk/gcc/autodep-cc srcs objs opts)
  (set! srcs  (augmk/group-norm srcs))
  (set! objs  (augmk/group-norm objs))
  (set! opts  (augmk/expand opts))
  (map
   (lambda (src obj)
     (augmk/gcc/autodep-helper (augmk/expand "$(CXX)") src obj opts))
   srcs objs))  
