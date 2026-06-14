	.z80
	title	CHGBNK - Bank switching module for the ASCII16 mapper
;
;-----------------------------------------------------------------------
;
; Manufacturer-supplied bank switching module for ROMs using the ASCII16
; mapper. Pasted at the tail of every local 16K bank of the Nextor ROM
; (offset 7FD0h..7FFFh in each bank) and called by the kernel whenever
; it needs to switch the page-1 ROM bank.
;
; Page 1 (4000h-7FFFh) on the ASCII16 mapper is one 16KB bank, selected
; directly by writing the bank number to the register at 6000h.
;
; Entry:  A = Nextor bank number to switch to
; Exit:   None
; Modifies: AF only
;
;-----------------------------------------------------------------------
;

BNKREG	equ	6000h	;ASCII16 bank register for 4000h-7FFFh
BNKID	equ	40FFh	;Address that holds the current bank ID

	org 7FD0h

CHGBNK:

	ld	(BNKREG),a
	ret
;
	defs	8000h-$,0FFh
;
	end
