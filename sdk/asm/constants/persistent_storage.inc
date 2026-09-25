;-----------------------------------------------------------------------------
;
; Persistent storage: the operations of the _PSOPS function call, and the
; layout of the data that Nextor keeps at the very beginning of the storage.
;
; See "Persistent storage" in the Nextor Programmers Reference for the full
; explanation, including the rules that a program modifying this data must
; follow (keep the size field and every byte it doesn't know about, and
; recalculate the checksum).
;
;-----------------------------------------------------------------------------

	include ../macros/const.inc

;Operations for the _PSOPS function call (in register A).

	const	PSOPS_GET_INFO,  0	;Get information about the storage
	const	PSOPS_READ,      1	;Read sectors from the storage
	const	PSOPS_WRITE,     2	;Write sectors to the storage

;Layout of the information returned by the PSOPS_GET_INFO operation.

	const	PSI_SLOT,          0	;Slot of the driver that provides it
	const	PSI_FLAGS,         1	;Flags, currently always zero
	const	PSI_DEVICE,        2	;Device holding the file, never zero
	const	PSI_SECTOR_SIZE,   3	;Sector size in bytes, 1 to 512 (2 bytes)
	const	PSI_SECTOR_COUNT,  5	;Number of sectors, at least 1 (2 bytes)
	const	PSI_LENGTH,        7	;Size of the information block

;In Nextor 3.0 the storage is always a file in a device, so PSI_FLAGS is zero,
;PSI_DEVICE is never zero, PSI_SECTOR_SIZE is 512 and PSI_SECTOR_COUNT is 1. Read
;these fields instead of assuming those values, and don't choke on a device
;number of zero: a later version may keep the storage elsewhere, and would use
;a flag and a device number of zero to say so.

;Layout of the data that Nextor keeps at the beginning of the storage.
;Everything past PSD_MIN_SIZE bytes is free for programs to use.

	const	PSD_SIGNATURE,     0	;"NEXTOR", zero terminated (7 bytes)
	const	PSD_SIZE,          7	;Size of the data, this field and the
					; signature included (2 bytes)
	const	PSD_VERSION,       9	;Version of the data format
	const	PSD_KEYS,         10	;Boot key inverters, FFFFh if not set
					; (2 bytes, see PSD_KEYSM_* below)
	const	PSD_EMU_DEVICE,   12	;Persistent disk emulation: device
					; number, 0 or FFh if not set
	const	PSD_EMU_SECTOR,   13	;Persistent disk emulation: absolute
					; sector number (4 bytes)
	const	PSD_EMU_FLAGS,    17	;Persistent disk emulation: flags
	const	PSD_ITEMS,        18	;Items stored, see PSD_ITEM_* below
	const	PSD_SCREEN,       19	;Screen parameters (5 bytes): registers
					; 1 to 10 of block 2 of the clock chip,
					; two per byte, the odd one in the low
					; nibble (only if PSD_ITEM_SCREEN is set)
	const	PSD_MIN_SIZE,     25	;Size of the data in format version 1
	const	PSD_MAX_SIZE,    512	;The data must always fit in one sector

	const	PSD_FORMAT_VERSION, 1	;Current value of the PSD_VERSION field

;The keys that have an inverter bit, in the two bytes at PSD_KEYS:
;1 to 7 in the first one, SHIFT and CTRL in the second one.

	const	PSD_KEYSM_LOW,   11111110b
	const	PSD_KEYSM_HIGH,  00110000b

;Bit numbers in PSD_ITEMS: which of the optional items hold valid data.
;They are the same as the bits of the parameter of CALL SETSCREEN.

	const	PSD_ITEM_SCREEN, 0	;Screen parameters, at PSD_SCREEN
