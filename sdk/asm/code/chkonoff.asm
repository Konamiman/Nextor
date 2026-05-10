	title	Nextor SDK - CHKONOFF

;-----------------------------------------------------------------------------
;
; Check a string that must be either "on" or "off".
; If it is none of these, terminate program with "Invalid parameter" error.
;
; Input:  HL = Pointer to zero-terminated parameter
; Output: A=0, Z if "off"; A=FFh, NZ if "on"
; Corrupts AF, HL

	ifrel
	public CHKONOFF
	extrn .IPARM
	extrn _TERM
	endif

	ifdef COM_FILE
BDOS	equ 5
	else
BDOS	equ 0F37Dh
	endif

CHKONOFF:
	ld	a,(hl)
	or	32
	cp	"o"
	jp	nz,IPARM

	inc	hl
	ld	a,(hl)
	or	32
	cp	"f"
	jr	nz,NO_OFF
	inc	hl
	ld	a,(hl)
	or	32
	cp	"f"
	jp	nz,IPARM
	inc	hl
	ld	a,(hl)
	or	a
	jp	nz,IPARM
	ret

NO_OFF:
	cp	"n"
	jp	nz,IPARM
	inc	hl
	ld	a,(hl)
	or	a
	jp	nz,IPARM
	dec	a
	ret

IPARM:
	ld	b,.IPARM
	ld	c,_TERM
	jp	BDOS
