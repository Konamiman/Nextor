;-----------------------------------------------------------------------
;
; Bank switching code for Nextor ROMs.
;
; This code must be at most 48 bytes long and be placed at offset 3FD0h
; on each 16K bank of the ROM, it will run at address 7FD0h.
;
; Input:  A = Nextor bank number to switch to
; Output: None
; Modifies: AF
;
;-----------------------------------------------------------------------
;

BNKID		equ	40FFh	;Address that holds the current bank ID

	; TODO: If you remove the header below, remove the "-3"
	org 7FD0h-3

	; This header tells mknexrom that the mapper uses 8K banks, so that
	; it patches the ROM boot code accordingly (without the patch the
	; ROM doesn't boot on 8K bank mappers).
	;
	; TODO: keep it if your mapper uses 8K banks AND you build the ROM
	; with mknexrom (put the address that switches the first 8K half in
	; the DW); otherwise remove it and change the org above to 7FD0h.

	db	0FFh	;Header marker recognized by mknexrom, must be 0FFh
	dw	6000h	;Address that switches the low 8K half

CHGBNK:

	; TODO: What follows is the bank switching code for ASCII8 mappers,
	; adjust it to the mapper used by the target hardware.
	rlca			;A = N*2 (low ASCII8 bank)
	ld	(6000h),a
	inc	a		;A = N*2+1 (high ASCII8 bank)
	ld	(6800h),a
	ret

	; Pad with 0FFh so the file covers the full 48 bytes of mapper code
	; that mknexrom expects (51 bytes counting the header).
	defs	8000h-$,0FFh
;
	end
