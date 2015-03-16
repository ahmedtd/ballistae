;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Begin module (augmk base)
;;
;; This module holds general utility functions that augmk submodules use.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Define the module
(define-module (augmk base))

;; Required modules.
(use-modules (gnu make))
(use-modules (ice-9 optargs))
(use-modules (ice-9 popen))
(use-modules (ice-9 rdelim))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; General utility functions for creating and emitting GNU Make code snippets.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Evaluate string ARG as a makefile snippet.
(define-public (augmk/eval . args)
  (gmk-eval args))

(define-public (augmk/expand . args)
  (gmk-expand args))

(define (augmk/to-makefile-string-helper arg)
  (cond ((list? arg)   (apply augmk/to-makefile-string arg))
        ((string? arg) arg)
        ((#t)          (object->string arg))))

(define-public (augmk/to-makefile-string . args)
  (string-join (map augmk/to-makefile-string-helper args)))

;; Join ARGS into a single string, using object->string.
(define-public (augmk/paste . args)
  (string-join (map augmk/to-makefile-string args) ""))

;; Join ARGS into a single string, and gmake-expand the result.
(define-public (augmk/paste-expand . args)
  (gmk-expand (apply augmk/paste args)))

;; Join ARGS into a single string, and gmake-eval the result.
(define-public (augmk/paste-eval . args)
  (augmk/eval (apply augmk/paste args)))

;; Join MSGS into single string and print using gmake info function.
(define-public (augmk/info . msgs)
  (augmk/eval (augmk/paste "$(info " (apply augmk/paste msgs) ")")))

;; Join MSGS into single string and print using gmake error function.
(define-public (augmk/error . msgs)
  (augmk/eval (augmk/paste "$(error " (apply augmk/paste msgs) ")")))

;; Create a dereference for a make variable.
(define-public (augmk/deref-create var)
  (augmk/paste "$(" var ")"))

(define-public (augmk/deref-eval varname)
  (augmk/eval (augmk/deref-create varname)))

;; Create a gmake foreach that maps CODE over GROUP.
;;
;; Within CODE, reference the current element using $(elt).
(define-public (augmk/create-foreach groupname code)
  (augmk/paste "$(foreach elt," (augmk/deref-create groupname) "," code ")"))

(define-public (augmk/create-subst from to in)
  (augmk/paste "$(subst " from "," to "," in ")"))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Utility functions for working with groups.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Convert GROUP to its list representation.
;;
;; GROUP is expanded and then split into whitespace-delimited tokens.
;;
;; Parameters:
;;
;;   * GROUP (string): A gmake variable name to dereference and split.
(define-public (augmk/group->list group)
  (string-split (string-trim-both (augmk/expand group)) char-set:whitespace))

;; Ensure that GROUP is in list representation.
;;
;; Parameters:
;;
;;   * GROUP (string or list): The GROUP in question.
;;
;; Return value: (list) The list form of GROUP.
(define-public (augmk/group-norm group)
  (if (string? group)
      (augmk/group->list group)
      group))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Utility functions for working with command flags.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Utility functions for working with paths.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Drop EXT from PATH.
;;
;; Parameters:
;;
;;   * PATH (string): Path to a file that may or may not include an extension.
;;
;;   * EXT (string): The extension to drop from PATH.
;;
;; Return value (string): If PATH did not end in EXT, returns PATH.  If PATH did
;; end in ext, returns PATH with the EXT stripped off.
(define-public (augmk/dirbasename path ext)
  (augmk/paste (dirname path) "/" (basename path ext)))

;; Map PATH over EXT-GROUP.
;;
;; Parameters:
;;
;;   * PATH (string): The path to augment with extensions.
;;
;;   * EXT-GROUP (group): The group of extensions.
;;
;;   * EXT (optional string): An existing extension to remove from PATH.
;;
;; Return value (group): A group where element i consists of PATH (with EXT
;;   removed) concatenated with element i of EXT-GROUP.
(define*-public (augmk/ext-map path ext-group #:optional (ext ""))
  (set! ext-group (augmk/group-norm ext-group))
  (set! path (augmk/paste (dirname path) "/" (basename path ext)))
  ;; Replicate path to list so that we can use map.  Probably not the best way.
  (set! path (make-list (length ext-group) path))
  (map augmk/paste path ext-group))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions for moving into and out of subdirectories.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-public (augmk/enter subdir)
  (let ((cur-fpath (augmk/expand "$(augmk_d)"))
        (new-fpath '()))
    (if (string-null? cur-fpath)
        (set! new-fpath subdir)
        (set! new-fpath (augmk/paste cur-fpath "/" subdir)))
    (augmk/paste-eval "augmk_d := " new-fpath)
    (augmk/eval "include $(augmk_d)/Rules.mk")
    (augmk/paste-eval "augmk_d := " cur-fpath)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Functions for working with subprocesses.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Call shell command as a function returning a string.
(define-public (augmk/call-cmd cmd)
  (let* ((prog-inst (open-pipe cmd OPEN_READ))
         (text      (read-string prog-inst)))
    (close-pipe prog-inst)
    text))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Fancy clean targets
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Create a clean target from a target group.
;;
;; Clean targets have special support because they create one phony deletion
;; target per member in the specified target group.  Then, they emit the named
;; target with a dependency on all of the created phonies.
(define-public (augmk/create-clean-target name group)

  ;; Normalize group to list form.
  (set! group (augmk/expand group))
  (set! group (augmk/group-norm group))

  ;; The target group contains real files that the user wants to delete.  We use
  ;; a textual transformation to create a phony name for each real target.
  (let* ((phony-appender (lambda (filename) (augmk/paste filename "_phony")))
         (phonies-string (map phony-appender group)))

    ;; Declare every target in phonies-string as phony
    (augmk/paste-eval ".PHONY: " phonies-string)

    ;; The phonies are "made" by deleting the corresponding non-phony file.
    (augmk/paste-eval "
" phonies-string " : 
	rm -rf $(@:_phony=)")
              
    (augmk/paste-eval ".PHONY: " name )
    (augmk/paste-eval name " : " phonies-string)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Install targets
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Create a phony target that installs [group] into $(DESTDIR)/[relpath] with
;; mode [modestring].
;;
;; Returns that name of the phony so that you can do this:
;;
;;     INSTALL_TARGETS += $(guile (augmk/create-install ...))
(define-public (augmk/install-phony phony-name relpath modestring group)

  ;; Expand args, since they may have computed components.
  (set! phony-name (augmk/expand phony-name))
  (set! relpath (augmk/expand relpath))
  (set! modestring (augmk/expand modestring))
  (set! group (augmk/expand group))

  (augmk/paste-eval ".PHONY: " phony-name)

  (augmk/paste-eval "
"phony-name" : "group"
	install -d $(DESTDIR)"relpath"
	install -t $(DESTDIR)"relpath" -m '"modestring"' "group"
")

  phony-name)

(define-public (augmk/install-hdr-phony phony-name relpath group)
  (set! relpath (augmk/paste-expand "$(DEST_HDR_DIR)/" relpath))
  (augmk/install-phony phony-name relpath "a=r,u=rw" group))

(define-public (augmk/install-lib-phony phony-name relpath group)
  (set! relpath (augmk/paste-expand "$(DEST_LIB_DIR)/" relpath))
  (augmk/install-phony phony-name relpath "a=r,u=rw" group))

(define-public (augmk/install-bin-phony phony-name relpath group)
  (set! relpath (augmk/paste-expand "$(DEST_BIN_DIR)/" relpath))
  (augmk/install-phony phony-name relpath "a=rx,u=rwx" group))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Immediate code -- run as the user loads the module.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Many Augmk recipe templates require ONESHELL.
(augmk/eval ".ONESHELL: ")

;; DELETE_ON_ERROR is just a good idea.
(augmk/eval ".DELETE_ON_ERROR: ")

;; Set a sane default for DESTDIR.  If the user specifies it (make DESTDIR="..."
;; ...), then that location will be used instead.
(augmk/eval "DESTDIR := /")
(augmk/eval "DEST_LIB_DIR := /usr/lib/")
(augmk/eval "DEST_HDR_DIR := /usr/include/")
(augmk/eval "DEST_BIN_DIR := /usr/bin/")
