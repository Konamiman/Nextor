	.z80

	name('driver_routines')
	title	Nextor SDK - Public constants for the driver signature and jump table

	.sall

;-----------------------------------------------------------------------------
;
; Nextor v3 driver header: signature and jump table.
; This header structure is used by both RAM and ROM drivers.
;
; The signature must be the fixed zero-terminated string "NEXTORv3_DRIVER". 

	include ../macros/var.inc

ram_ad	defl	4100h

	var	SIGNATURE,16
	var3	TIMI_ENTRY,4110
	var3	BASSTAT_ENTRY,4113
	var3	BASDEV_ENTRY,4116
	var3	EXTBIO_ENTRY,4119
	var3	DRIVER_QUERY_ENTRY,411C
	var3	DEVICE_QUERY_ENTRY,411F
	var3	CUSTOM_DRIVER_QUERY_ENTRY,4122
	var3	CUSTOM_DEVICE_QUERY_ENTRY,4125
	var3	READ_WRITE_ENTRY,4128
	;The DIRECT*_ENTRY entries are only used by ROM drivers
	;and may be omitted from the jump table of RAM drivers.
	var3	DIRECT0_ENTRY,412B
	var3	DIRECT1_ENTRY,412E
	var3	DIRECT2_ENTRY,4131
	var3	DIRECT3_ENTRY,4134
	var3	DIRECT4_ENTRY,4137

	const	HEADER_END,ram_ad	;First address after the header
