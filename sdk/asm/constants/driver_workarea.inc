	.z80

	name('driver_workarea')
	title	Nextor SDK - Page 3 work area locations relevant to drivers
	subttl	Public constants for kernel/system work area addresses

	.sall

;-----------------------------------------------------------------------------
;
; Nextor page3 work area addresses that drivers are allowed (and sometimes need)
; to read or write directly. Standard MSX BIOS work area addresses are NOT
; included here, see msx_workarea.inc for those.
;
;-----------------------------------------------------------------------------

	include ../macros/const.inc

;-----------------------------------------------------------------------------
;
; Nextor-specific work area
;

;Address of routine to be called via CALDRV / C4PBK.
	const	BK4_ADD, 0F1D0h

;Temporary storage of IX/IY around CALLB0_IX_IY calls.
	const	TMP_IX,  0F1D2h
	const	TMP_IY,  0F1D4h

;Nextor version number (e.g. 30h for Nextor 3.0).
	const	NXT_VER, 0F318h

;Will be 0 in DOS 2 mode or 3 in DOS 1 mode.
	const	MAIN_BANK, 0F319h

;-----------------------------------------------------------------------------
;
; Work area inherited from MSX-DOS
;

;Table of disk driver slots.
	const	DRVTBL,  0FB21h

;DOS version number (high nibble = major, low nibble = minor).
;0 for MSX-DOS 1.
	const	DOS_VER, 0F313h

;Slot of the master driver currently being addressed.
	const	MASTER_SLOT, 0F348h


