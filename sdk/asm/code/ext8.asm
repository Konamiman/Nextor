	title	Nextor SDK - EXT8

;-----------------------------------------------------------------------------
;
; Extracts a 8 bit number from a string
;
; Input:  HL = ASCII string address
; Output: A  = number
;         E  = First non-numeric character
;         HL = Address after the number
;         Cy = 1 if error (not a number, or number too big)
; Modifies: BC

	ifrel
	public EXT8
	extrn EXTNUM
	endif

EXT8:
	call EXTNUM
	ret	c
	or	a
	scf
	ret	nz
	ld	a,b
	or	a
	scf
	ret	nz
	ld	a,d
	or	a
	scf
	ret	z
	ld	a,c
	ld	c,d
	ld	b,0
	add	hl,bc
	or	a
	ret
