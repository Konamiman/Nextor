	.z80

	name('dos_file_handles')
	title	Nextor SDK - Standard file handles
	subttl	Public constants for the standard DOS file handles

	.sall

;-----------------------------------------------------------------------------
;
; Standard DOS file handle numbers.
;
;-----------------------------------------------------------------------------

	include ../macros/const.inc

	const	STDIN,  0		;Standard input channel
	const	STDOUT, 1		;Standard output channel
	const	STDERR, 2		;Standard error channel
	const	STDAUX, 3		;Auxilliary I/O channel
	const	STDLST, 4		;Standard list channel


