	.z80

;-----------------------------------------------------------------------------
;
; Standard MSX system work area addresses commonly accessed by Nextor drivers.
;
; Reference: MSX2 Technical Handbook, Appendix 4 ("Work area listing")
; https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Appendix4.md
;
;-----------------------------------------------------------------------------

;Screen width for SCREEN 0 (1 byte).
LINL40	equ	0F3AEh

;Screen width for SCREEN 1 (1 byte).
LINL32	equ	0F3AFh

;Current screen width in characters (1 byte).
LINLEN	equ	0F3B0h

;Number of rows on current screen (1 byte).
CRTCNT	equ	0F3B1h

;Current cursor Y position (1-based).
CSRY	equ	0F3DCh

;Current cursor X position (1-based).
CSRX	equ	0F3DDh

;Function-key display flag (0 = hidden).
CNSDFG	equ	0F3DEh

;BASIC line input buffer; commonly reused as scratch space.
BUF	equ	0F55Eh

;Type of the most recent expression evaluated by FRMEVL.
;Values: 2=integer, 3=string, 4=single-precision, 8=double-precision.
VALTYP	equ	0F663h

;Result accumulator used by the BASIC numeric routines (FRMEVL, FRESTR, ...).
DAC	equ	0F7F6h

;BASIC "newline only" flag (set during program load to suppress output).
NLONLY	equ	0F87Ch

;BASIC "string mode" flag for the line buffer.
FLBMEM	equ	0FCAEh

;Current screen mode (0..8).
SCRMOD	equ	0FCAFh

;Expanded slot flags (one byte per main slot).
EXPTBL	equ	0FCC1h

;Attributes for each slot.
SLTATR	equ	0FCC9h

;Work area address for each slot (4 bytes per slot).
SLTWRK	equ	0FD09h

;Name of the BASIC procedure being parsed by a CALL command (16 bytes).
PROCNM	equ	0FD89h

;CHPUT extension hook.
H_CHPH	equ	0FDA4h
