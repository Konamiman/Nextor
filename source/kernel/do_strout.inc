;-----------------------------------------------------------------------------
;
; do_strout - emit an inline STROUT implementation (ESC-Y bug corrected).
;
; The macro body is defined here, not in macros.inc, because it references
; _CONOUT and the caller must therefore have it defined (either via an
; `include ../../sdk/asm/constants/dos_calls.inc' or via an `extrn _CONOUT'
; declaration). Files that just include macros.inc don't pull this in unless
; they actually need it.
;
; Arguments:
;   OUTDIR  - routine used to print a character.
;   RETDIR  - routine jumped to at end. If 0, a RET is generated instead.
;   LOADCE  - if non-blank, E and C registers are loaded prior to calling
;             OUTDIR (E = char, C = _CONOUT).
;
;-----------------------------------------------------------------------------

do_strout	macro	OUTDIR,RETDIR,LOADCE
	local	stro_init
	local	STROU2
	local	ST10

stro_init:
	LD	A,(DE)
	INC	DE
	cp	ESC
	jr	nz,STROU2

	call	ST10		;Print 'ESC'
	ld	a,(DE)
	inc	de
	cp	'Y'
	jr	nz,STROU2

	call	ST10		;Print 'Y'
	ld	a,(DE)
	inc	de
	call	ST10		;Print X coordinate
	ld	a,(DE)
	inc	de
	call	ST10		;Print Y coordinate

	jr	stro_init

STROU2:
	CP	'$'
	ifidn	<RETDIR>,<0>
	RET	Z
	else
	JP	Z,&RETDIR&
	endif
	call	ST10
	JR	stro_init

ST10:	PUSH	DE

	ifnb	<LOADCE>
	LD	E,A
	LD	C,_CONOUT
	endif

	CALL	&OUTDIR&
	POP	DE

	RET

	endm
