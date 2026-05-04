	title	Nextor SDK - EXTPAR

	.Z80

;-----------------------------------------------------------------------------
;
; Extract a parameter from the command line
;
; Input:   A  = Parameter index (starting at 1)
;          DE = Destination address for parameter
; Output:  A  = Total number of parameters
;          CY = 1 -> Parameter does not exists
;                    B undefined, destination buffer unmodified
;          CY = 0 -> Parameter copied at address DE, zero terminated
;                    B = Parameter length (not including terminating zero)
; Modifies: -

	ifrel
	public EXTPAR
	endif

EXTPAR:	or	a	;Terminate with error if A = 0
	scf
	ret	z

	ld	b,a
	ld	a,(80h)	;Terminate with error if no parameters available
	or	a
	scf
	ret	z
	ld	a,b

	push	af
	push	hl
	ld	a,(80h)
	ld	c,a	;Put zero at the end of command line
	ld	b,0
	ld	hl,81h
	add	hl,bc
	ld	(hl),0
	pop	hl
	pop	af

	push	hl
	push	de
	push	ix
	ld	ix,0	;IXl: Number of parameters
	ld	ixh,a	;IXh: Parameter to be extracted
	ld	hl,81h

PASASPC:
	ld	a,(hl)	;Skip spaces
	or	a
	jr	z,ENDPNUM
	cp	" "
	inc	hl
	jr	z,PASASPC

	inc	ix
PASAPAR:	ld	a,(hl)	;Traverse parameter characters
	or	a
	jr	z,ENDPNUM
	cp	" "
	inc	hl
	jr	z,PASASPC
	jr	PASAPAR

ENDPNUM:
	ld	a,ixh	;Error if parameter index
	dec	a	;is larger that the number of parameters
	cp	ixl
	jr	nc,EXTPERR

	ld	hl,81h
	ld	b,1	;B = current parameter
PASAP2:	ld	a,(hl)	;Skip spaces until finding next parameter
	cp	" "
	inc	hl
	jr	z,PASAP2

	ld	a,ixh	;If it is the desired parameter, extract it.
	cp	B	;Otherwise...
	jr	z,PUTINDE0

	inc	B
PASAP3:	ld	a,(hl)	;...skip it and jump to PASAP2
	cp	" "
	inc	hl
	jr	nz,PASAP3
	jr	PASAP2

PUTINDE0:
	ld	b,0
	dec	hl
PUTINDE:
	inc	b
	ld	a,(hl)
	cp	" "
	jr	z,ENDPUT
	or	a
	jr	z,ENDPUT
	ld	(de),a	;Copy paramater to (DE)
	inc	de
	inc	hl
	jr	PUTINDE

ENDPUT:	xor	a
	ld	(de),a
	dec	b

	ld	a,ixl
	or	a
	jr	FINEXTP
EXTPERR:
	scf
FINEXTP:
	pop	ix
	pop	de
	pop	hl
	ret
