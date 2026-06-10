; @@NAME@@ — a Nextor-aware MSX-DOS command (scaffolded by nextor-init).
;
; A minimal .COM that prints a banner and exits cleanly. It is built against
; the Nextor SDK, so you can extend it with DOS / Nextor function calls using
; the constants in $NEXTOR_SDK/asm/constants/ and the snippets in asm/code/.

	.z80

BDOS	equ	0005h		;MSX-DOS function dispatcher (.COM entry path)

	INCLUDE asm/constants/dos_calls.inc

	org	0100h

START:
	ld	de,BANNER
	ld	c,_STROUT	;print a '$'-terminated string
	call	BDOS

	ld	c,_TERM0	;terminate, return code 0
	jp	BDOS

BANNER:
	db	"@@NAME@@ - built with the Nextor SDK.",13,10,"$"

	end		;a .COM always starts at 0100h, so no entry-point argument

; Next steps:
;  - To act only when running under Nextor, assemble in the SDK's Nextor check:
;    see $NEXTOR_SDK/asm/code/chk_nextor.asm.
;  - DOS / Nextor function numbers are in asm/constants/dos_calls.inc;
;    error codes in dos_errors.inc; BIOS/work-area addresses in msx_*.inc.
