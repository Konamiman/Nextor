;-----------------------------------------------------------------------------
;
; Fixed-address entry points and data items in the Nextor ROM bank header
; (the 4000h-40FFh area, identical in every kernel bank) plus the bank
; switching routine. ROM drivers may use these as they see fit.
;
;-----------------------------------------------------------------------------

	include ../macros/const.inc

;-----------------------------------------------------------------------------
;
; * GSLOT1 - Get in A the current slot for page 1. Corrupts F.
;   Must be called by using CALBNK to bank 0:
;   xor a
;   ld ix,GSLOT1
;   call CALBNK
;
	const	GSLOT1,        402Dh


;-----------------------------------------------------------------------------
;
; * RDBANK - Read a byte from another bank.
;   Must be called by using CALBNK to the desired bank, passing the
;   address to be read in HL:
;   ld a,bank
;   ld hl,address
;   ld ix,RDBANK
;   call CALBNK
;
	const	RDBANK,        403Ch


;-----------------------------------------------------------------------------
;
; * CALLB0 - Switch to the kernel main bank (bank 0 in DOS 2 mode, bank 3
;   in DOS 1 mode), call the routine whose address is in BK4_ADD, then
;   restore the original bank.
;
;   ROM drivers can use this directly. RAM drivers should use CALLB0_IX_IY
;   instead, since CALLB0 expects IX to already hold the right value
;   (it is loaded from BK4_ADD before the call).
;
;   Input:  (BK4_ADD) = routine address
;           AF, BC, DE, HL, IY passed to the routine
;
	const	CALLB0,        403Fh


;-----------------------------------------------------------------------------
;
; * CALBNK - Call a routine in another bank.
;   Must be used if the driver spans more than one bank.
;   Input: A = bank
;          IX = routine address
;          AF' = AF for the routine
;          BC, DE, HL, IY = input for the routine
;
	const	CALBNK,        4042h


;-----------------------------------------------------------------------------
;
; * GWORK - Get in IX the address of the SLTWRK entry for the slot in A,
;   which contains a pointer to the allocated page-3 work area for that
;   slot (0 if no work area was allocated).
;   If A=0, uses the slot currently switched in page 1; returns A = current
;   slot for page 1. Corrupts F.
;   Must be called by using CALBNK to bank 0:
;   ld a,slot
;   ex af,af'
;   xor a
;   ld ix,GWORK
;   call CALBNK
;
	const	GWORK,         4045h


;-----------------------------------------------------------------------------
;
; * CALLB0_IX_IY - Same as CALLB0, but takes IX and IY from page 3 RAM
;   (TMP_IX and TMP_IY) so that the caller can pass them through CALSLT
;   without losing them. Required for RAM drivers, since the interslot
;   call corrupts IX/IY on the way in.
;
;   Input:  (BK4_ADD) = routine address
;           AF, BC, DE, HL passed to the routine
;           (TMP_IX) = IX for the routine
;           (TMP_IY) = IY for the routine
;
	const	CALLB0_IX_IY,  404Bh


;-----------------------------------------------------------------------------
;
; * K_SIZE - Address of a one-byte field that tells how many banks form
;   the Nextor kernel (also the first bank number of the driver).
;
	const	K_SIZE,        40FEh


;-----------------------------------------------------------------------------
;
; * CUR_BANK - Address of a one-byte field that holds the current bank
;   number paged in at page 1.
;
	const	CUR_BANK,      40FFh


;-----------------------------------------------------------------------------
;
; * CHGBNK - Bank switching routine, located at the end of the driver
;   bank (7FD0h). Selects the bank whose number is in A.
;
	const	CHGBNK,        7FD0h
