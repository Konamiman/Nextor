	title	Nextor SDK - GETSLOT_NOTERM

	.Z80

;-----------------------------------------------------------------------------
;
; Extract a slot+segment from a string with the format <slot>[-<subslot>][:<segment>]
; (non-terminating variant).
;
; Input: HL = String address
; Output: Cy = 0 on success: A = Slot, B = Segment
;         Cy = 1 if the string is not valid (A and B undefined)
; Modifies: AF, BC, DE, HL

	ifrel
	public GETSLOT_NOTERM
	extrn EXT8
	endif

GETSLOT_NOTERM:
	ld	a,-1
	ld	(SUBSLOT),a
	ld	(SEGMENT),a

	call	EXT8	;Get main slot number
	ret	c
	cp	4
	ccf
	ret	c
	ld	(SLOT),a
	ld	a,e
	or	a
	jr	z,OK_SLOTSUBSEG
	cp	":"
	jr	z,OK_SUBSLOT
	cp	"-"
	jr	nz,ERROR

	inc	hl
	call	EXT8	;Get subslot number
	ret	c
	cp	4
	ccf
	ret	c
	ld	(SUBSLOT),a
	ld	a,e
	or	a
	jr	z,OK_SLOTSUBSEG
	cp	":"
	jr	nz,ERROR
OK_SUBSLOT:

	inc	hl
	call	EXT8	;Get segment number
	ret	c
	ld	(SEGMENT),a
	ld	a,e
	or	a
	jr	nz,ERROR
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
	or	a
	ret

ERROR:
	scf
	ret

SLOT:
	db	0
SUBSLOT:
	db	0
SEGMENT:
	db	0
