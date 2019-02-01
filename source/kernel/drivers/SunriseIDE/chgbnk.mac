	.z80
	title	CHGBNK - Bank Switching Module for the Sunrise IDE mapper
;
;-----------------------------------------------------------------------
;
;   This is a manufacturer-supplied bank switching module.   This module
; is placed at the tail of every local banks of DOS2-ROM.
;
;   This is a sample  program.   DOS2-ROM  has  no  assumptions  on  the
; mechanism  of bank switching, for example, where the bank register is,
; which bits are assigned to bank switching,  etc.   The  bank  register
; does not have to be readable.
;
; Entry:  Acc = 0 --- switch to bank #0
;		1 --- switch to bank #1
;		2 --- switch to bank #2
;		3 --- switch to bank #3
; Exit:   None
;
; Only AF can be modified
;
; *** CODE STRTS HERE ***	CAUTION!!  This must be the first module.
;
BNKREG	equ	6000h		;System IC version
BNKID	equ	40FFh		;Where Bank ID is stored

;Bank change for Sunrise ID, it is quite weird:
;bank bit 0 -> register bit 7
;bank bit 1 -> register bit 6
;bank bit 2 -> register bit 5
;bank bit 3 -> register bit 4
;bank bit 2 -> register bit 3

CHGBNK::
	push	bc
	srl	a
	rl	b
	srl	a
	rl	b
	srl	a
	rl	b
	srl	a
	rl	b
	srl	a
	rl	b

	ld	a,b
	pop	bc
	rlca
	rlca
	rlca
	and	11111000b

	ld	(4104h),a
	ret
;
	defs	(8000h-7FD0h)-($-CHGBNK),0FFh
;
	end
