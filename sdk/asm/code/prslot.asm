	title	Nextor SDK - PRSLOT

	.Z80

;-----------------------------------------------------------------------------
;
; Print a slot+segment number
; The output will be <slot>[-<subslot>][:<segment>]
;
; Input: A = Slot number, B = Segment number
; Modifies: AF, BC, DE, HL

	ifrel
	public PRSLOT
	extrn _CONOUT
	extrn _ZSTROUT
	extrn BYTE2ASC	
	
	endif

PRSLOT:
	push	bc
	push	af
	and	11b
	add	a,"0"
	ld	e,a
	ld	c,_CONOUT
	call	5		;Print main slot number

	pop	af
	bit	7,a
	jr	z,DO_PRINT_SEGMENT
	rrca
	rrca
	and	11b
	add	a,"0"
	ld	e,a
	push	de
	ld	e,"-"
	ld	c,_CONOUT
	call	5
	pop	de
	ld	c,_CONOUT
	call	5		;Print sub-slot number

DO_PRINT_SEGMENT:
	pop	bc
	ld	a,b
	cp	0FFh
	ret	z

	push	ix
	ld	ix,PRSLOT_BUF+1
	call	BYTE2ASC
	ld	(ix),0
	pop	ix
	ld	de,PRSLOT_BUF
	ld	c,_ZSTROUT
	jp	5

PRSLOT_BUF:
	db	":000",0
