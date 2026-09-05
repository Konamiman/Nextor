; Jump table entries for bank 6 public routines.
; Included by both bank6/jump.mac and data.mac.
; The jentry, ventry and signature macros must be defined before including this file.

; Nextor 2 compat driver entries

	signature "NEXTORv2_COMPAT"
	ventry	V2_NOT_IMPLEMENTED	;4110h: TIMER_INT
	ventry	V2_NOT_IMPLEMENTED	;4113h: OEMSTAT
	ventry	V2_NOT_IMPLEMENTED	;4116h: BASDEV
	ventry	V2_NOT_IMPLEMENTED	;4119h: EXTBIO
	ventry	V2_DRIVER_QUERY		;411Ch: DRIVER_QUERY
	ventry	V2_DEVICE_QUERY		;411Fh: DEVICE_QUERY
	ventry	V2_NOT_IMPLEMENTED	;4122h: CUSTOM_DRIVER_QUERY
	ventry	V2_NOT_IMPLEMENTED	;4125h: CUSTOM_DEVICE_QUERY
	ventry	V2_READ_WRITE		;4128h: READ_WRITE
	rept	8			;Reserved and direct-call entries
	ventry	V2_NOT_IMPLEMENTED
	endm

; Other entries

	jentry	SETUP_GHOST_DOS2
	jentry	SETUP_GHOST_DOS1
	jentry	GHOST_PROMPT_DOS2
	jentry	PRINT_KERNEL_MSG
	jentry	BUILD_FMT_CHOICES
	jentry	F_FORMAT
	jentry	UNDO_GHOST_DRVTBL
	jentry	FMT_PRINT_AND_CHOOSE
	jentry	DSK_CHOICE_BUF
	jentry	IDRV_PRINT_OK
	jentry	UDRV_PRINT_OK
	jentry	IDRV_AUTOMAP
	jentry	CIDRIVER
	jentry	CUDRIVER
