	.z80

;-----------------------------------------------------------------------------
;
; Standard MSX system work area addresses commonly accessed by Nextor drivers.
;
; Reference: MSX2 Technical Handbook, Appendix 4 ("Work area listing")
; https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Appendix4.md
;
;-----------------------------------------------------------------------------

;Type of the most recent expression evaluated by FRMEVL.
;Values: 2=integer, 3=string, 4=single-precision, 8=double-precision.
VALTYP	equ	0F663h

;Result accumulator used by the BASIC numeric routines (FRMEVL, FRESTR, ...).
DAC	equ	0F7F6h

;BASIC line input buffer; commonly reused as scratch space.
BUF	equ	0F55Eh

;BASIC "newline only" flag (set during program load to suppress output).
NLONLY	equ	0F87Ch

;BASIC "string mode" flag for the line buffer.
FLBMEM	equ	0FCAEh

;Expanded slot flags (one byte per main slot).
EXPTBL	equ	0FCC1h

;Attributes for each slot.
SLTATR	equ	0FCC9h

;Work area address for each slot (4 bytes per slot).
SLTWRK	equ	0FD09h

;Name of the BASIC procedure being parsed by a CALL command (16 bytes).
PROCNM	equ	0FD89h
