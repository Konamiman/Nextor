;-----------------------------------------------------------------------------
;
; MSX BASIC interpreter ROM routines that are useful when implementing
; new BASIC CALL commands. All addresses are in the BASIC interpreter ROM
; (page 1, "MAIN" slot); they must be invoked via CALBAS (BIOS, 0159h)
; so that the interpreter ROM is paged in correctly.
;
; Reference: MSX2 Technical Handbook, Chapter 2 section 4.4
; ("Expansion of CMD command").
; https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Chapter2.md#44-expansion-of-cmd-command
;
;-----------------------------------------------------------------------------

;Execute one BASIC text statement
NEWSTT	equ	4601h

;Extract one character from BASIC text (skipping spaces)
CHRGTR	equ	4666h

;Evaluate an expression in BASIC text
FRMEVL	equ	4C64h

;Evaluate an expression as a 1-byte integer (0..255)
GETBYT	equ	521Ch

;Evaluate an expression as a 2-byte integer
FRMQNT	equ	542Fh

;Look up a variable in the symbol table (allocating it if missing)
PTRGET	equ	5EA4h

;Register a string value (used together with FRMEVL when evaluating strings)
FRESTR	equ	67D0h
