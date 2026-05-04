	title	Nextor SDK - PRPAD

	.Z80

;-----------------------------------------------------------------------------
;
; Prints a string which is padded with spaces at right
; (modifies the string by appending a 0 byte over the first padding space)
;
; Input: HL = String address
;        BC = String length

	ifrel
	public PRPAD
	extrn _ZSTROUT
	endif

PRPAD:
	push	hl
	add	hl,bc
PRPAD_LOOP:
	dec	hl
	ld	a,(hl)
	cp	" "
	jr	nz,PRPAD_FOUND
	dec	bc
	ld	a,b
	or	c
	jr	nz,PRPAD_LOOP
	ret	;Do nothing if the string was just spaces

PRPAD_FOUND:
	inc	hl
	ld	(hl),0

	pop	de
	ld	c,_ZSTROUT
	jp	5
