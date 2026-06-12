	.z80
	title	CHGBNK - Bank Switching Module for the ASCII8 mapper
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
BNKREG	equ	6000h		;System IC version
BNKID	equ	40FFh		;Where Bank ID is stored

;Bank change for Sunrise ID, it is quite weird:
;bank bit 0 -> register bit 7
;bank bit 1 -> register bit 6
;bank bit 2 -> register bit 5

	; The 3 byte header below is consumed by mknexrom (it doesn't end up
	; in the ROM), so starting 3 bytes early makes the bank switching code
	; itself assemble exactly at 7FD0h, the address it will run at.
	org 7FD0h-3

	db	0FFh	;Header for MKNEXROM
	dw	6000h

CHGBNK:

	rlca
	ld	(6000h),a
	inc	a
	ld	(6800h),a
	ret
;
	defs	8000h-$,0FFh
;
	end
