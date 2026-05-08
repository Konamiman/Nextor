	title	Nextor SDK - BYTE2ASC

;-----------------------------------------------------------------------------
;
; Convert a 8 bit number to an unterminated string
;
; Input:  A  = Number
;         IX = Destination address
; Output: IX points after the generated string
; Modifies: C

	ifrel
	public BYTE2ASC
	endif

BYTE2ASC:	cp	10
	jr	c,B2A_1D
	cp	100
	jr	c,B2A_2D
	cp	200
	jr	c,B2A_1XX
	jr	B2A_2XX

	;--- One digit

B2A_1D:	add	a,"0"
	ld	(ix),a
	inc	ix
	ret

	;--- Two digits

B2A_2D:	ld	c,"0"
B2A_2D2:
	inc	c
	sub	10
	cp	10
	jr	nc,B2A_2D2

	ld	(ix),c
	inc	ix
	jr	B2A_1D

	;--- Between 100 and 199

B2A_1XX:
	ld	(ix),"1"
	sub	100
B2A_XXX:
	inc	ix
	cp	10
	jr	nc,B2A_2D	;If 1XY with X>0
	ld	(ix),"0"	;If 10Y
	inc	ix
	jr	B2A_1D

	;--- Between 200 and 255

B2A_2XX:
	ld	(ix),"2"
	sub	200
	jr	B2A_XXX
