	title	Nextor SDK - GETSLOT

	.Z80

;-----------------------------------------------------------------------------
;
; Extract a slot+segment from a string with the format <slot>[-<subslot>][:<segment>]
; If the string is not valid, terminate with "Invalid parameter" error.
;
; Input: HL = String address
; Output: A = Slot, B = Segment
; Modifies: AF, BC, DE, HL

	ifrel
	public GETSLOT
	extrn EXT8
	extrn .IPARM
	extrn _TERM
	endif

GETSLOT:
	ld	a,-1
	ld	(SUBSLOT),a
	ld	(SEGMENT),a

	call	EXTNUM_TERM	;Get main slot number
	cp	4
	jp	nc,IPARM
	ld	(SLOT),a
	ld	a,e
	or	a
	jr	z,OK_SLOTSUBSEG
	cp	":"
	jr	z,OK_SUBSLOT
	cp	"-"
	jp	nz,IPARM

	inc	hl
	call	EXTNUM_TERM	;Get subslot number
	cp	4
	jp	nc,IPARM
	ld	(SUBSLOT),a
	ld	a,e
	or	a
	jr	z,OK_SLOTSUBSEG
	cp	":"
	jp	nz,IPARM
OK_SUBSLOT:

	inc	hl
	call	EXTNUM_TERM	;Get segment number
	ld	(SEGMENT),a
	ld	a,e
	or	a
	jp	nz,IPARM
OK_SLOTSUBSEG:

	;* Convert slot+subslot to slot byte

	ld	a,(SUBSLOT)
	cp	-1
	jr	z,OK_MAKESLOT

	rlca
	rlca
	and	00001100b
	ld	b,a
	ld	a,(SLOT)
	or	b
	or	10000000b
	ld	(SLOT),a
OK_MAKESLOT:

	ld	a,(SEGMENT)
	ld	b,a
	ld	a,(SLOT)
	ret

SLOT:
	db	0
SUBSLOT:
	db	0
SEGMENT:
	db	0


EXTNUM_TERM:
	call	EXT8
	ret	nc
IPARM:
	ld	b,.IPARM
	ld	c,_TERM
	jp	5
