	title	Nextor SDK - CHKLET_NOTERM

;-----------------------------------------------------------------------------
;
; Check a parameter that must be a drive letter (non-terminating variant).
;
; Input:  HL = Pointer to zero-terminated parameter
; Output: A=Drive letter (1=A:, 2=B:, etc) on success;
;         A=.IPARM (invalid parameter error code) on failure
; Corrupts AF, HL

	ifrel
	public CHKLET_NOTERM
	extrn .IPARM
	endif

CHKLET_NOTERM:
	ld	a,(hl)
	or	32
	cp	"a"
	jr	c,CHKLET4
	cp	"h"+1
	jr	nc,CHKLET4
	push	af
	inc	hl
	ld	a,(hl)
	cp	":"
	jr	nz,CHKLET3
	inc	hl
	ld	a,(hl)
	or	a
	jr	nz,CHKLET3

	pop	af
	sub	"a"-1
	ret

CHKLET3:
	pop	af
CHKLET4:
	ld	a,.IPARM
	ret
