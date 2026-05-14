	.z80

;-----------------------------------------------------------------------------
;
; MSX BIOS entry points commonly used by Nextor drivers.
;
; Reference: MSX2 Technical Handbook, Appendix 1 ("BIOS listing")
; https://github.com/Konamiman/MSX2-Technical-Handbook/blob/master/md/Appendix1.md
;
;-----------------------------------------------------------------------------

;Inter-slot read: A = slot, HL = addr -> A = byte
RDSLT	equ	000Ch

;Compare HL with DE (sets Z if equal, C if HL < DE)
DCOMPR	equ	0020h

;Inter-slot write: A = slot, HL = addr, E = byte
WRSLT	equ	0014h

;Inter-slot call: address in IYh:IX
CALSLT	equ	001Ch

;Enable a slot permanently
ENASLT	equ	0024h

;Console input (returns char in A)
CHGET	equ	009Fh

;Console output (A = char)
CHPUT	equ	00A2h

;Check Control-STOP key (returns Z=1 if pressed)
CKCNTC	equ	00BDh

;Call a routine in the BASIC interpreter ROM (address in IX)
CALBAS	equ	0159h

;Extended BIOS entry point.
;Call with D = device number, E = function number; other registers as required.
EXTBIO	equ	0FFCAh
