;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Begin module (augmk tex)
;;
;; The TeX ecosystem seems like it was deliberately designed to maximize the
;; difficulty of working with it in makefiles.  It's not possible to directly
;; specify inputs and outputs like most compilers.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Define the module.
(define-module (augmk tex))

;; Required modules.
(use-modules (gnu make))
(use-modules (augmk base))
(use-modules (ice-9 optargs))

;; For each document.tex in SOURCES, adds tex-ecosystem auxiliary
;; output to target-group GROUP.
;;
;; Parameters:
;;
;;   * PATH: Path (including .tex extension) to the main source file for the
;;     document.
;;
;; Return Value (group): The files (relative to top project dir) to clean.
(define-public (augmk/tex/pdflatex-clean maintex)
  (augmk/ext-map maintex "$(AUGMK_TEX_CLEAN_EXTENSIONS)" ".tex"))

;; Emit a recipe to produce paper.pdf from paper.tex
;;
;; Parameters:
;;
;;   * MAINTEX (string): Full path to the main source file of a document
;;   (including the .tex extension).
;;
;; This procedure has no return value, but rather creates and then
;; evaluates some gmake code.  After defining the target, the user can
;; add additional dependencies to the corresponding pdf target.
(define-public (augmk/tex/pdflatex-target maintex)
  (augmk/paste-eval "
" (augmk/paste (augmk/dirbasename maintex ".tex") ".pdf") " : " maintex "
	pushd " (dirname maintex) "
	pdflatex -shell-escape -halt-on-error "(basename maintex ".tex")"
	if grep \"Please (re)run Biber on the file\" "(basename maintex ".tex")".log
	then
	    biber --quiet "(basename maintex ".tex")"
	    pdflatex -shell-escape -halt-on-error "(basename maintex ".tex")"
	fi
	if grep \"Label(s) may have changed\" "(basename maintex ".tex")".log
	then
	    pdflatex -shell-escape -halt-on-error "(basename maintex ".tex")"
	fi
	popd
"))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Immediate code -- run as the user loads the module.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Dump a list of "TeX junk" extensions so that the user can modify it.
(define augmk/tex/exts-to-clean '(".aux"
                                  ".bbl"
                                  ".bcf"
                                  ".blg"
                                  ".log"
                                  ".out"
                                  ".run.xml"
                                  ".pyg"
                                  ".tex.blg"))
(augmk/eval "AUGMK_TEX_CLEAN_EXTENSIONS := " augmk/tex/exts-to-clean)
