	title	Nextor SDK - CHK_NEXTOR

;-----------------------------------------------------------------------------
;
; Checks that Nextor 3 is running and that NEXTOR.SYS 2.0 is loaded.
; If so, returns normally; otherwise, terminates program.
; Corrupts AF, BC, DE.

	ifrel
	public CHK_NEXTOR
	extrn _DOSVER
	extrn _STROUT
	extrn _TERM0
	endif

	ifdef COM_FILE
BDOS	equ 5
	else
BDOS	equ 0F37Dh
	endif

CHK_NEXTOR:
	ld	b,05Ah
	ld	hl,01234h
	ld	de,0ABCDh
	ld	c,_DOSVER
	ld	ix,0
	call	BDOS
	push	de

	ld	de,BADKER_MSG
	ld	a,b
	cp	2
	jr	c,CHK_NEXTOR_ERR
	push	ix
	pop	bc
	ld	a,b
	cp	1	;NEXTOR_ID
	jr	nz,CHK_NEXTOR_ERR
	ld	a,c
	cp	3
	jr	c,CHK_NEXTOR_ERR

	pop	bc
	ld	de,BADSYS_MSG
	ld	a,b
	cp	2
	ret	nc

CHK_NEXTOR_ERR:
	ld	c,_STROUT
	call	BDOS
	ld	c,_TERM0
	jp	BDOS

BADKER_MSG:
	db	"*** This program requires Nextor 3.0 or later",13,10,"$"
BADSYS_MSG:
	db	"*** Bad version of NEXTOR.SYS, version 2.0 or later is required",13,10,"$"
