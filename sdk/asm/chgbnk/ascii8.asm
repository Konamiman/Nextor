	.z80
	title	CHGBNK - Bank switching module for the ASCII8 mapper
;
;-----------------------------------------------------------------------
;
; Manufacturer-supplied bank switching module for ROMs using the ASCII8
; mapper. Pasted at the tail of every local 16K bank of the Nextor ROM
; (offset 7FD0h..7FFFh in each bank) and called by the kernel whenever
; it needs to switch the page-1 ROM bank.
;
; Page 1 (4000h-7FFFh) on the ASCII8 mapper is made of two consecutive
; 8KB ASCII8 banks: the low half (4000h-5FFFh) is selected via the
; register at 6000h and the high half (6000h-7FFFh) via the register at
; 6800h. To page in Nextor 16K bank N, the low half is set to N*2 and the
; high half to N*2+1.
;
; Entry:  A = Nextor bank number to switch to
; Exit:   None
; Modifies: AF only
;
;-----------------------------------------------------------------------
;

BNKREG_LO	equ	6000h	;ASCII8 bank register for 4000h-5FFFh
BNKREG_HI	equ	6800h	;ASCII8 bank register for 6000h-7FFFh
BNKID		equ	40FFh	;Address that holds the current bank ID

	; The 3 byte header below is consumed by mknexrom (it doesn't end up
	; in the ROM), so starting 3 bytes early makes the bank switching code
	; itself assemble exactly at 7FD0h, the address it will run at.
	org 7FD0h-3

	db	0FFh	;Header bytes consumed by mknexrom: marker
	dw	BNKREG_LO	;and address of the bank register

CHGBNK:

	rlca			;A = N*2 (low ASCII8 bank)
	ld	(BNKREG_LO),a
	inc	a		;A = N*2+1 (high ASCII8 bank)
	ld	(BNKREG_HI),a
	ret
;
	defs	8000h-$,0FFh
;
	end
