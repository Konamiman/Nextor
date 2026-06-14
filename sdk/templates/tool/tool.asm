; Nextor tool template - a Nextor-aware .COM program.
;
; A minimal .COM that prints a banner and exits cleanly. It builds as-is;
; make it yours by following the TODO comments. It is built against the
; Nextor SDK, so you can extend it with DOS / Nextor function calls using
; the constants in the SDK's asm/constants/ and the snippets in asm/code/.

BDOS	equ	0005h		;MSX-DOS function dispatcher (.COM entry path)

	INCLUDE asm/constants/dos_calls.inc

	org	0100h

START:
	;TODO: replace the banner printing with your tool's actual work.
	ld	de,BANNER
	ld	c,_STROUT	;print a '$'-terminated string
	call	BDOS

	ld	c,_TERM0	;terminate, return code 0
	jp	BDOS

BANNER:
	;TODO: your tool's name here.
	db	"mytool - built with the Nextor SDK.",13,10,"$"

	end		;a .COM always starts at 0100h, so no entry-point argument

; Next steps:
;  - To act only when running under Nextor, assemble in the SDK's Nextor check:
;    see asm/code/chk_nextor.asm in the SDK.
;  - DOS / Nextor function numbers are in asm/constants/dos_calls.inc;
;    error codes in dos_errors.inc; BIOS/work-area addresses in msx_*.inc.
