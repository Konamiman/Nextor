;
;        This file is included by all the major COMMAND3.COM source files.
; It defines the MSX-DOS and BIOS entry points and workarea addresses used
; by the program, and the offsets of the control words kept in the mapper
; segment that holds the command history and the alias space.
;
;        The DOS function and error codes are not defined here: they are
; resolved at link time from the SDK constant files (sdk/asm/constants),
; assembled with GENERATE_PUBLIC_SYMBOLS (see the Makefile).
;
;        Based on COMMAND.COM 2.31, copyright (1986) IS Systems Ltd, and on
; the COMMAND2.COM 2.40/2.41 additions by Fokke Post, whose source code was
; released as freeware in 2003.
;

;------- MSX-DOS and BIOS entry points
;
BDOS:	EQU	#05	; MSX-DOS function call entry point.
RDSLT:	EQU	#0C	; Inter-slot read (also available under MSX-DOS).
CALSLT:	EQU	#1C	; Inter-slot call (also available under MSX-DOS).
WRTVDP:	EQU	#47	; Write VDP register (main BIOS, via CALSLT).

;------- MSX-DOS kernel variables
;
ERR_TO_BASIC:	EQU	#F245	; DOS error code that the Nextor kernel
			;   reports at the next entry to BASIC.
BOOTDRV:	EQU	#F2FD	; Drive MSX-DOS boots from when leaving BASIC.
DATEFMT:	EQU	#F30E	; Country code (determines the date format).

;------- BIOS workarea variables
;
LINL40:	EQU	#F3AE	; Columns in SCREEN 0.
LINLEN:	EQU	#F3B0	; Columns in the current screen mode.
CRTCNT:	EQU	#F3B1	; Rows in the current screen mode.
CSRY:	EQU	#F3DC	; Cursor row (1 based).
CSRX:	EQU	#F3DD	; Cursor column (1 based).
FORCLR:	EQU	#F3E9	; Foreground colour.
BAKCLR:	EQU	#F3EA	; Background colour.
BDRCLR:	EQU	#F3EB	; Border colour.
RG9SAV:	EQU	#FFE8	; Shadow of VDP register 9.
RTYCNT:	EQU	#FC9A	; BIOS workarea byte repurposed to remember the
			;   history/alias mapper segment across re-loads.
EXPTBL:	EQU	#FCC1	; Slot expanded flags (and main BIOS slot).
SLTWRK:	EQU	#FD88	; Slot workarea table.
EXTBIO:	EQU	#FFCA	; Extended BIOS hook.

;------- Mapper segment control words
;
;        COMMAND3.COM keeps the command history, the alias list and the
; variable swap area in a RAM mapper segment, switched into page 2 while
; it is accessed. These are the offsets of the 16-bit control words kept
; at the bottom of that segment, so each one lives at offset+#8000 (see
; the accessor routines in editor.mac and the alias routines in io.mac).
;
RDADDR:	EQU	#0000	; History buffer read (cursor) position.
PUTADDR:	EQU	RDADDR+#02	; History buffer write position.
ENDADDR:	EQU	PUTADDR+#02	; End of the history data.
BUFLEN:	EQU	ENDADDR+#02	; Size of the history buffer.
VAREND:	EQU	BUFLEN+#02	; End of the variable swap area.
ALIADDR:	EQU	VAREND+#02	; Start of the alias list.
ALIEND:	EQU	ALIADDR+#02	; End of the alias list.
MAGICW:	EQU	ALIEND+#02	; "C2" once the segment has been
			;   initialised: RTYCNT remembers the
			;   segment number across re-loads and
			;   is a repurposed BIOS byte, so its
			;   value is only trusted when the
			;   segment carries this mark.
BUFADDR:	EQU	MAGICW+#02	; Start of the free buffer area.
