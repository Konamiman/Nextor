; Jump table entries for bank 6 public routines.
; Included by both bank6/jump.mac and data.mac.
; The jentry macro must be defined before including this file.

	jentry	SETUP_GHOST_DOS2
	jentry	SETUP_GHOST_DOS1
	jentry	GHOST_PROMPT_DOS2
	jentry	PRINT_KERNEL_MSG
	jentry	F_FORMAT
	jentry	UNDO_GHOST_DRVTBL
	jentry	FMT_PRINT_AND_CHOOSE
	jentry	DSK_CHOICE_BUF
	jentry	CIDRIVER
	jentry	CUDRIVER
	jentry	PS_BOOT
	jentry	PS_GET_EMU
	jentry	F_PSOPS
	jentry	CBOOTKEYS
	jentry	CEMUKILL
	jentry	CALL_EXTRA
	jentry	CSETSCREEN
	jentry	PS_SCREEN
	jentry	F_FMT
	jentry	NEW_UPB6
	jentry	F_BFIN
	jentry	CN_LINE6
