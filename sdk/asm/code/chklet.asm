	title	Nextor SDK - CHKLET

;-----------------------------------------------------------------------------
;
; Check a parameter that must be a drive letter.
; If it is not a valid drive letter, terminate program with "Invalid parameter" error.
;
; Input:  HL = Pointer to zero-terminated parameter
; Output: A=Drive letter (1=A:, 2=B:, etc)
; Corrupts AF, HL

	ifrel
	public CHKLET
	extrn .IPARM
	extrn _TERM
	endif

	ifdef COM_FILE
BDOS	equ 5
	else
BDOS	equ 0F37Dh
	endif

CHKLET:
	ld	a,(hl)
	or	32
	cp	"a"
	jp	c,IPARM
	cp	"h"+1
	jp	nc,IPARM
	push	af
	inc	hl
	ld	a,(hl)
	cp	":"
	jp	nz,IPARM
	inc	hl
	ld	a,(hl)
	or	a
	jp	nz,IPARM

	pop	af
	sub	"a"-1
	ret

IPARM:
	ld	b,.IPARM
	ld	c,_TERM
	jp	BDOS
