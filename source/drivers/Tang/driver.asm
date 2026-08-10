	.z80
	title	MSX-DOS 2 Copyright (1986)  IS Systems Ltd.
	subttl	WonderTANG uSD Driver Kernel

;-----------------------------------------------------------------------------

; Driver version

VER_MAIN		equ	1
VER_SEC			equ	0
VER_REV			equ	0

;-----------------------------------------------------------------------------
;
; SD Controller registers and bit definitions

SDC_ENABLE		equ	7E00h		; wo: 1: enable SDC register, 0: disable
SDC_CMD			equ	SDC_ENABLE+1	; wo: cmd to SDC fpga: 1=read, 2=write
SDC_STATUS		equ	SDC_CMD+1	; ro: SDC status bits
SDC_SADDR		equ	SDC_STATUS+1	; wo: 4 bytes: sector addr for read/write
SDC_C_SIZE		equ	SDC_SADDR+4	; ro: 3 bytes: device size blocks
SDC_C_SIZE_MULT		equ	SDC_C_SIZE+3	; ro: 3 bits size multiplier
SDC_RD_BL_LEN		equ	SDC_C_SIZE_MULT+1	; ro: 4 bits block length
SDC_CTYPE		equ	SDC_RD_BL_LEN+1	; ro: SDC Card type: 0=unknown, 1=SDv1, 2=SDv2, 3=SDHCv2
SDC_MID			equ	SDC_CTYPE+1
SDC_OID			equ	SDC_MID+1
SDC_PNM			equ	SDC_OID+2
SDC_PSN			equ	SDC_PNM+5
SDC_CRC16		equ	SDC_PSN+4

SDC_SDATA		equ	7C00h		; rw: 7C00h-7Dff - sector transfer area

SDC_BUSY		equ	080h
SDC_CRC			equ	001h
SDC_TIMEOUT		equ	002h

SDC_READ		equ	001h
SDC_WRITE		equ	002h
SDC_INIT		equ	080h

;-----------------------------------------------------------------------------
;
; Standard BIOS and work area entries

CHPUT			equ	00A2h		; Character output
CHGET			equ	009Fh
INITXT			equ	006Ch
CLS			equ	0848H
MSXVER			equ	002DH
LINL40			equ	0F3AEh		; Width
LINLEN			equ	0F3B0h
;------------------------------------------------------
;
; Work area definition
;
; +0-3: Device size in sectors
; +4-7: current sector r/w
;-----------------------------------------------------------------------------

;-----------------------------------------------------------------------------

	include	../../sdk/asm/constants/driver_result_codes.inc

	module	DRIVER_QUERY
	include	../../sdk/asm/constants/driver_driver_queries.inc
	endmod

	module	DEVICE_QUERY
	include	../../sdk/asm/constants/driver_device_queries.inc
	endmod

	include	../../sdk/asm/constants/dos_errors.inc

	include	../../sdk/asm/constants/rom_bank_header.inc


;*********************
;***  DRIVER CODE  ***
;*********************

	org	4100h

DRIVER_START:

; Driver signature

	db	"NEXTORv3_DRIVER",0

; Jump table

	jp	TIMER_INT
	jp	OEMSTAT
	jp	BASDEV
	jp	EXTBIO
	jp	DRIVER_QUERY
	jp	DEVICE_QUERY
	jp	CUSTOM_DRIVER_QUERY
	jp	CUSTOM_DEVICE_QUERY
	jp	READ_WRITE
	jp	RESERVED_0
	jp	RESERVED_1
	jp	RESERVED_2
	jp	DIRECT_0
	jp	DIRECT_1
	jp	DIRECT_2
	jp	DIRECT_3
	jp	DIRECT_4

	ds	4180h-$,0

STR_DRIVER_NAME:
	db	"WonderTANG! uSD Driver",0
STR_DRIVER_AUTHOR:
	db	"Luis Antoniosi",0

;-----------------------------------------------------------------------------
;
; Timer interrupt routine, it will be called on each timer interrupt
; (at 50 or 60Hz), but only if DRV_INIT returns Cy=1 on its first execution.

TIMER_INT:
	ret

;-----------------------------------------------------------------------------
;
; BASIC expanded statement ("CALL") handler.
; Works the expected way, except that CALBAS in kernel page 0
; must be called instead of CALBAS in MSX BIOS.

OEMSTAT:
	scf
	ret


;-----------------------------------------------------------------------------
;
; BASIC expanded device handler.
; Works the expected way, except that CALBAS in kernel page 0
; must be called instead of CALBAS in MSX BIOS.

BASDEV:
	scf
	ret

;--- Extended BIOS hook.
;    Works the expected way, except that it must return
;    IYl=1 if the old hook must be called, IYl=0 otherwise.
;    Only called if the driver has returned EXTBIO flag set
;    in the "get driver initialization parameters" query.
EXTBIO:
	ret
	ret
	ret

; * Jump table entries reserved for future use.

RESERVED_0:
RESERVED_1:
RESERVED_2:
	ret

; * Direct calls entry points.
;  There is a jump table at address 7850h in ROM banks 0 and 3,
;  that will be redirected here.

DIRECT_0:
	ret
	ret
	ret

DIRECT_1:
	ret
	ret
	ret

DIRECT_2:
	ret
	ret
	ret

DIRECT_3:
	ret
	ret
	ret

DIRECT_4:
	ret
	ret
	ret

;--- Driver query
;    Input:  A = Query index
;            F, BC, DE, HL = Depends on the query
;    Output: A = Error code:
;                RESULT_OK: success
;                RESULT_NOT_IMPLEMENTED: query not implemented
;                Others: depends on the query
;            F, BC, DE, HL = Depends on the query

DRIVER_QUERY:
	dec	a
	jp	z,DO_DRVQ_GET_VERSION
	dec	a
	jp	z,DO_DRVQ_GET_STRING
	dec	a
	jp	z,DO_DRVQ_GET_INIT_PARAMS
	dec	a
	jp	z,DO_DRVQ_INIT
	dec	a
	jp	z,DO_DRVQ_GET_MAX_DEVICE
;	dec
;	jp
	ld	a,RESULT_NOT_IMPLEMENTED
	ret

; Driver query 1: Get driver version number
;
; Input:  A = 1
; Output: A = RESULT_OK or RESULT_NOT_IMPLEMENTED
;         Version in B.C.D (if A=RESULT_OK)
;
; Note: Same as the Nextor 2 DRV_VERSION, but version is now returned
; in B.C.D instead of A.B.C, and an error code is returned in A.

DO_DRVQ_GET_VERSION:
	ld	b,VER_MAIN
	ld	c,VER_SEC
	ld	d,VER_REV
	xor	a
	ret

; Driver query 2: Get driver information string
;
; Input:  B  = String index:
;              1: Driver name
;              2: Driver author name
;              3: Hardware name
;              4: Hardware author name
;              5: Serial number
;        D  = Buffer size
;        HL = Buffer address
; Output: A = RESULT_OK: ok, full string provided
;             RESULT_TRUNCATED_STRING: string was truncated due to buffer size too short
;             RESULT_NOT_IMPLEMENTED: requested string not available
;
; String is always provided zero-terminated, so the max effective string length is 254.

DO_DRVQ_GET_STRING:
	ld	a,b
	dec	b
	jp	z,DRIVER_NAME
	dec	b
	jp	z,DRIVER_AUTHOR
	dec	b
	jp	z,DEVICE_NAME
	ld	a,RESULT_NOT_IMPLEMENTED
	ret
DRIVER_NAME:
	ld	b,d
	ex	de,hl
	ld	hl,STR_DRIVER_NAME
	jp	OUTPUT_STRING
DRIVER_AUTHOR:
	ld	b,d
	ex	de,hl
	ld	hl,STR_DRIVER_AUTHOR
	jp	OUTPUT_STRING
DEVICE_NAME:
	ld	b,d
	ex	de,hl
	ld	hl,STR_DEVICE_NAME
	jp	OUTPUT_STRING

; Driver query 3: Get driver initialization parameters
;
; Input:  HL = Amount of work area available to allocate
;         B  = Number of available drives in the system (???)
;         C  = Flags:
;              5: set if user is requesting reduced drive count (by pressing the 5 key)
;              Others: 0
;         DE = Address of a routine for printing a character
; Output: A  = RESULT_OK, RESULT_INIT_ERROR or RESULT_NOT_IMPLEMENTED
;         B  = Flags
;              0: TIMER_INT should be hooked
;              1: EXTBIO should be hooked
;              2-7: Must be zero
;         HL = Space required in page 3
;
; If RESULT_NOT_IMPLEMENTED is returned, B=0 and HL=0 is assumed.
; RESULT_INIT_ERROR will cause the "initialize driver" call to be skipped
; and the driver to be ignored (not counted as an existing Nextor kernel).
;
; Note: this is the same as Nextor 2 DV_INIT when called with A=0, except that
; TIMER_INT flag is returned in B, not in Cy; an error code is returned in A;
; and DE is passed at input.

DO_DRVQ_GET_INIT_PARAMS:
	ld	a,RESULT_OK
	ld	b,0
	ld	hl,0
	ret

; Driver query 4: Initialize driver
;
; Input:  HL = Amount of work area allocated for the driver in page 3
;         C  = Flags:
;              5: set if user is requesting reduced drive count (by pressing the 5 key)
;              Others: 0
;         DE = Address of a routine for printing a character
; Output: A  = RESULT_OK, RESULT_INIT_ERROR or RESULT_NOT_IMPLEMENTED
;
; RESULT_NOT_IMPLEMENTED is interpreted as equivalent to RESULT_OK.
; RESULT_INIT_ERROR will cause the driver to be ignored (not counted as an existing Nextor kernel).
;
; Note: this is the same as Nextor 2 DV_INIT when called with A=1, except that number of
; allocated drives is not passed in B, DE is passed at input, and an error code can be returned.

DO_DRVQ_INIT:
;	ld
;	cp
;	jr
;	ld
;	jr
;MSX1:
;	ld	a,40
;SETSCRN:
;	ld
;	call	INITXT

	ld	de,INFO_S
	call	PRINT

	ld	de,SEARCH_S
	call	PRINT

	call	MY_GWORK
	call	SDC_ON
	ld	a,SDC_INIT
	ld	(SDC_CMD),a

	ld	(ix),0				; clear device data

WAIT_RESET:
	ld	de,2047				; Timeout
WAIT_RESET1:
	ld	a,0
	cp	e
	jr	nz,WAIT_DOT			; Print dots while waiting
;	ld	a,46
;	call	CHPUT
WAIT_DOT:
	call	CHECK_ESC
	jp	c,INIT_NO_DEV
	ld	b,255
WAIT_RESET2:
	ld	a,(SDC_STATUS)
	and	SDC_BUSY
	jr	z,WAIT_RESET_END		; Wait for BSY to clear and DRDY to set
	djnz	WAIT_RESET2
	dec	de
	ld	a,d
	or	e
	jr	nz,WAIT_RESET1
	jp	INIT_NO_DEV
WAIT_RESET_END:

	ld	a,(SDC_CTYPE)
	or	a
	jp	z,INIT_NO_DEV

	ld	de,SDV1
	cp	1
	jr	z,PRINT_CTYPE
	ld	de,SDV2
	cp	2
	jr	z,PRINT_CTYPE
	ld	de,SDHCV2
	cp	3
	jr	z,PRINT_CTYPE
	ld	de,STR_UNKNOWN

PRINT_CTYPE:					; print card type
	call	PRINT

	ld	de,CRLF_S
	call	PRINT

	ld	hl,SDC_C_SIZE
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	c,(hl)				; c:de = c_size
	inc	hl

	ld	hl,1
	add	hl,de
	ex	de,hl

	ld	a,0
	adc	a,c				; c:de = c_size + 1
	ld	c,a
	jr	nc,NO_OVL
	ld	a,1				; overflow
	jr	OVL
NO_OVL:
	xor	a
OVL:

	ex	af,af'				; preserve msb
	push	af

	ld	a,(SDC_CTYPE)
	cp	3
; SDHC ignores c_size_mult and read_bl_len: each c_size unit is
; 512 KiB, or 1024 fixed-size 512-byte sectors.
	ld	b,10
	jr	z,SHIFT_SIZE

	ld	hl,SDC_C_SIZE_MULT
	ld	b,(hl)				; b = c_size_mult
	inc	hl
	inc	b
	inc	b				; b = c_size_mult + 2

	ld	a,(hl)				; a = read_bl_len

	add	a,b
	sub	9
	jr	z,NO_SHIFT
	ld	b,a				; b = read_bl_len + c_size_mult + 2 - 9

SHIFT_SIZE:
	pop	af
	ex	af,af'				; restore msb
CALC_SIZE:
	sla	e
	rl	d
	rl	c
	rl	a
	djnz	CALC_SIZE
	jr	ST_SIZE
NO_SHIFT:
	pop	af
	ex	af,af'				; restore msb

ST_SIZE:

	ld	(ix+0),e			; store card size in sectors (512 bytes) in work area (32-bit)
	ld	(ix+1),d
	ld	(ix+2),c
	ld	(ix+3),a

	ld	de,CRLF_S
	call	PRINT

;;;;

; wait some time
	ld	b,0
outer:
	push	bc
	ld	b,0
inner:
	nop
	push	ix
	pop	ix
	djnz	inner
	pop	bc
	djnz	outer

	jr	DRV_INIT_END

INIT_NO_DEV:
	call	CHECK_ESC
	jr	c,INIT_NO_DEV

	ld	de,CRLF_S
	call	PRINT
	ld	de,NODEVS_S
	call	PRINT

	xor	a
	ld	(ix+0),a
	ld	(ix+1),a
	ld	(ix+2),a
	ld	(ix+3),a

	call	SDC_OFF
	ld	a,RESULT_INIT_ERROR
	ret

;--- End of the initialization procedure
DRV_INIT_END:
	call	SDC_OFF
	ld	a,RESULT_OK
	ret

; Driver query 5: Get maximum supported device number
;
; Input:  -
; Output: A = RESULT_OK or RESULT_NOT_IMPLEMENTED
;         B = Maximum supported device number
;
; RESULT_NOT_IMPLEMENTED is equivalent to returning RESULT_OK and B=4.

DO_DRVQ_GET_MAX_DEVICE:
	ld	a,RESULT_OK
	ld	b,1
	ret

; Driver query 6: Initialize RAM driver
;
; Input:  DE = Address of a routine for printing a character
; Output: A  = RESULT_OK, RESULT_INIT_ERROR or RESULT_NOT_IMPLEMENTED
;        B  = Flags
;             0: TIMER_INT should be hooked
;             1: EXTBIO should be hooked
;             2-7: Must be zero
;
; If RESULT_NOT_IMPLEMENTED is returned, B=0 is assumed.
;
;
; Driver query 7: Shutdown RAM driver
;
; Input:  DE = Address of a routine for printing a character
; Output: A  = RESULT_OK or RESULT_NOT_IMPLEMENTED
;
;
; Queries 6 and 7 not implemented as this is a ROM driver,
; these queries will never be invoked by the kernel.


;--- Device query
;    Input:  A = Query index
;            C = Device number
;            F, B, DE, HL = Depends on the query
;    Output: A = Error code:
;                RESULT_OK: success
;                RESULT_INVALID_DEVICE: Invalid device number
;                RESULT_NOT_IMPLEMENTED: query not implemented
;                Others: depends on the query
;            F, BC, DE, HL = Depends on the query

DEVICE_QUERY:
	dec	c
	jr	nz,INVALID_DEVICE
	dec	a
	jp	z,DO_DEVQ_GET_STRING
	dec	a
	jp	z,DO_DEVQ_GET_PARAMS
	dec	a
	jp	z,DO_DEVQ_GET_STATUS
	dec	a
	jp	z,DO_DEVQ_GET_AVAILABILITY
	dec	a
	jp	z,DO_DEVQ_GET_FORMAT_CHOICES
	dec	a
	jp	z,DO_DEVQ_DO_FORMAT
	dec	a
	jp	z,DO_DEVQ_STOP_MOTOR
	ld	a,RESULT_NOT_IMPLEMENTED
	ret
INVALID_DEVICE:
	ld	a,RESULT_INVALID_DEVICE
	ret

; Device query 1: Get device information string
;
; Input:  B  = String index:
;              1: Manufacturer name
;              2: Medium name
;              3: Serial number
;              4: Device name
;         D  = Buffer size
;         HL = Buffer address
; Output: A = RESULT_OK: ok, full string provided
;            RESULT_TRUNCATED_STRING: string was truncated due to buffer size too short
;            RESULT_INVALID_DEVICE: device does not exist
;            RESULT_NOT_IMPLEMENTED: requested string not available
;
; String is always provided zero-terminated, so the max effective string length is 254.
;
; Note: this is the same as Nextor 2 DEV_INFO (minus B=0 at input), but device id
; is passed in C instead of A, there's the buffer size parameter, and error codes differ.

DO_DEVQ_GET_STRING:
	dec	b
	jp	z,MANUFACTURER
	dec	b
	jp	z,MEDIUM_NAME
	dec	b
	jp	z,SERIAL_NUMBER
	dec	b
	jp	z,DEVICE_NAME2
	ld	a,RESULT_NOT_IMPLEMENTED
	ret
MANUFACTURER:
	ld	b,d
	ex	de,hl
	ld	hl,STR_UNKNOWN
	jp	OUTPUT_STRING
MEDIUM_NAME:
	ld	b,d
	ex	de,hl
	call	SDC_ON
	ld	a,(SDC_CTYPE)
	ex	af,af'
	call	SDC_OFF
	ex	af,af'
	ld	hl,SDV1
	cp	1
	jr	z,DEVDONE
	ld	hl,SDV2
	cp	2
	jr	z,DEVDONE
	ld	hl,SDHCV2
	cp	3
	jr	z,DEVDONE
DEVUNK:
	ld	hl,STR_UNKNOWN
DEVDONE:
	jp	OUTPUT_STRING

STR_PAD:
	xor	a
	or	b
	jr	z,DEV_INFO_OK
STR_PAD_LOOP:
	xor	a
	ld	(hl),a
	inc	hl
	djnz	STR_PAD_LOOP
DEV_INFO_OK:
	call	SDC_OFF
	ld	a,RESULT_OK
	ret
SERIAL_NUMBER:
	call	SDC_ON
	ld	a,(SDC_PSN+3)
	call	BINTOHEX
	dec	d
	jp	z,ERR_TRUNCATED
	ld	(hl),b
	inc	hl
	dec	d
	jp	z,ERR_TRUNCATED
	ld	(hl),c
	inc	hl
	ld	a,(SDC_PSN+2)
	call	BINTOHEX
	dec	d
	jp	z,ERR_TRUNCATED
	ld	(hl),b
	inc	hl
	dec	d
	jp	z,ERR_TRUNCATED
	ld	(hl),c
	inc	hl
	ld	a,(SDC_PSN+1)
	call	BINTOHEX
	dec	d
	jp	z,ERR_TRUNCATED
	ld	(hl),b
	inc	hl
	dec	d
	jp	z,ERR_TRUNCATED
	ld	(hl),c
	inc	hl
	ld	a,(SDC_PSN+0)
	call	BINTOHEX
	dec	d
	jp	z,ERR_TRUNCATED
	ld	(hl),b
	inc	hl
	dec	d
	jp	z,ERR_TRUNCATED
	ld	(hl),c
	inc	hl
	ld	b,d
	jp	STR_PAD

ERR_TRUNCATED:
	call	SDC_OFF
	ld	a,RESULT_TRUNCATED_STRING
	ret
DEVICE_NAME2:
	call	SDC_ON
	ld	c,d
	ex	de,hl
	ld	hl,SDC_OID
	ld	b,7
COPNAME:
	ld	a,(hl)
	cp	32
	jr	c,NONASC
	cp	128
	jr	nc,NONASC
	jr	STORASC
NONASC:
	ld	a,' '
STORASC:
	dec	c
	jr	z,ERR_TRUNCATED
	ld	(de),a
	inc	de
	inc	hl
	djnz	COPNAME
	ex	de,hl
	ld	b,c
	jp	STR_PAD

; Device query 2: Get device parameters
;
; Input:  HL = Buffer address, 0 for not returning info (only return error code)
; Output: A =  RESULT_OK: ok, device information provided
;              RESULT_INVALID_DEVICE: device does not exist
;              RESULT_NOT_IMPLEMENTED: query not implemented
;
; On success, buffer filled with the following information:
;
; +0 (1): Device type:
;         0: Block device
;         1: CD or DVD reader or recorder
;         2-254: Unused. Additional codes may be defined in the future.
;         255: Other
; +1 (2): Sector size, 0 if this information does not apply or is
;         not available.
; +3 (4): Total number of available sectors.
;         0 if this information does not apply or is not available.
; +7 (1): Flags:
;         bit 0: 1 if the device is removable.
;         bit 1: 1 if the device is read only. A device that can dinamically
;                  be write protected or write enabled is not considered
;                  to be read-only.
;         bit 2: 1 if the device is a floppy disk drive.
;         bit 3: 1 if this device shouldn't be used for automapping.
;         bits 4-7: must be zero.
; +8 (2): Number of cylinders
; +10 (1): Number of heads
; +11 (1): Number of sectors per track
;
; RESULT_NOT_IMPLEMENTED is interpreted as a block device with 512 byte sectors, unknown total number of sectors, and flags equal to 0.
;
; This is the same as Nextor 2 LUN_INFO, but device id is passed in C instead of A, there's no LUN index, and error codes differ.
; Also HL=0 at input must be supported.

DO_DEVQ_GET_PARAMS:

	ld	a,h
	or	l
	jr	nz,FILL_INFO
	ld	a,RESULT_OK
	ret

FILL_INFO:
	call	MY_GWORK

	xor	a
	ld	(hl),a				; block device
	inc	hl
	ld	(hl),0
	inc	hl
	ld	(hl),2				; sector size
	inc	hl
	ld	a,(ix+0)
	ld	(hl),a
	inc	hl
	ld	a,(ix+1)
	ld	(hl),a
	inc	hl
	ld	a,(ix+2)
	ld	(hl),a
	inc	hl
	ld	a,(ix+3)
	ld	(hl),a				; total of sectors
	inc	hl
	xor	a
	ld	(hl),a				; medium flags
	inc	hl
	ld	(hl),a				;
	inc	hl
	ld	(hl),a				; num of cyls
	inc	hl
	ld	(hl),a				; num of heads
	inc	hl
	ld	(hl),a				; num of sectors per track

	ld	a,RESULT_OK
	ret

; Device query 3: Get device status
;
; Input: -
; Output: A = RESULT_OK: ok, device information provided
;             RESULT_INVALID_DEVICE: device does not exist
;             RESULT_NOT_IMPLEMENTED: query not implemented or device isn't removable
;         B = Status for the specified device:
;             0: The device exists but is not available at the moment
;                (typically this means: removable device with no medium inserted)
;             1: The device is available and has not
;                changed since the last status request.
;             2: The device is available and has changed
;                since the last status request
;             3: The device is available, but it is not
;                possible to determine whether it has been changed
;                or not since the last status request.
;
; RESULT_NOT_IMPLEMENTED is interpreted as returning B=1.
;
; For fixed devices the routine can return either RESULT_NOT_IMPLEMENTED, or RESULT_OK and B=1.
;
; This is the same as Nextor 2 DEV_STATUS, but device id is passed in C instead of A, there's no LUN index, and error codes differ.
; Also the behavior when input is a non existing device is different (previously it would return a status of 0, now it returns RESULT_INVALID_DEVICE).

DO_DEVQ_GET_STATUS:
DEV_CHECK_SIZE:
	call	MY_GWORK
	ld	a,(ix+0)
	or	a
	jr	nz,DEV_OK
	ld	a,(ix+1)
	or	a
	jr	nz,DEV_OK
	ld	a,(ix+2)
	or	a
	jr	nz,DEV_OK
	ld	a,(ix+3)
	or	a
	jr	nz,DEV_OK
DEV_STAT_ERR:
	ld	a,RESULT_INVALID_DEVICE
	ret

DEV_OK:
	ld	a,RESULT_OK
	ld	b,3
	ret

; Device query 4: Get device availability
;
; Input: -
; Output: A = RESULT_OK: ok, device information provided
;             RESULT_INVALID_DEVICE: device does not exist
;             RESULT_NOT_IMPLEMENTED: query not implemented or device isn't removable
;         B = Status for the specified device:
;             0: The device exists but is not available at the moment
;                (typically this means: removable device with no medium inserted)
;             1: The device is available
;
; RESULT_NOT_IMPLEMENTED is interpreted as retruning B=1.
;
; Note: this is the same as "Get device status" but it only returns B=0 or B=1,
; and it does not change the internal "changed" status of the device.

DO_DEVQ_GET_AVAILABILITY:
	call	DEV_CHECK_SIZE
	ld	b,1
	cp	RESULT_OK
	ret	z
	ld	b,0
	ret

; Device query 5: Get format choices for a floppy disk device
;
; Input:  DE = Buffer size (used if B=255 is returned)
;         HL = Buffer address (used if B=255 is returned)
; Output: A = RESULT_OK: ok, format information provided
;             RESULT_INVALID_DEVICE: device does not exist
;             RESULT_NOT_IMPLEMENTED: not a floppy disk,
;                                    or formatting not supported
;             RESULT_TRUNCATED_STRING: string was truncated due to buffer size too short
;         B = Choices:
;             0: Only one format choice available
;             1: Single side / double side, double density
;             2: Single side / double side DD / double side HD
;             255: Driver has written a custom null-terminated choice
;                  string to the buffer at HL
;

DO_DEVQ_GET_FORMAT_CHOICES:
	ld	a,RESULT_NOT_IMPLEMENTED
	ret

; Device query 6: Format a floppy disk device
;
; Input:  B  = Choice number (1-9, as chosen by user from choice string)
; Output: A = RESULT_OK: ok, disk has been formatted
;             RESULT_INVALID_DEVICE: device does not exist
;             RESULT_NOT_IMPLEMENTED: the device is not a floppy disk,
;                                    formatting is not supported,
;                                    or the choice number is invalid.
;
; The driver should format the floppy disk according to the selected choice.
; Choice numbers correspond to the format choices returned by query 5.
;
; Disk parameters (MSX-DOS 1 compatible boot sector, FAT, root directory)
; must be initialized by this routine upon succesful formatting.

DO_DEVQ_DO_FORMAT:
	ld	a,RESULT_NOT_IMPLEMENTED
	ret

; Device query 7: Stop the floppy disk drive motor
;
; Input:  -
; Output: RESULT_OK: ok, motor has been stopped
;         RESULT_INVALID_DEVICE: device does not exist
;         RESULT_NOT_IMPLEMENTED: the device is not a floppy disk
;                                or stopping the drive motor is not supported

DO_DEVQ_STOP_MOTOR:
	ld	a,RESULT_NOT_IMPLEMENTED
	ret

;--- Custom driver query
;    Input:  A = Query index
;            F, BC, DE, HL = Depends on the query
;    Output: A = Error code:
;                RESULT_OK: success
;                RESULT_NOT_IMPLEMENTED: query not implemented
;                Others: depends on the query
;            F, BC, DE, HL = Depends on the query

CUSTOM_DRIVER_QUERY:
	ld	a,RESULT_NOT_IMPLEMENTED
	ret


;--- Custom device query
;    Input:  A = Query index
;            F, BC, DE, HL = Depends on the query
;    Output: A = Error code:
;                RESULT_OK: success
;                RESULT_NOT_IMPLEMENTED: query not implemented
;                Others: depends on the query
;            F, BC, DE, HL = Depends on the query

CUSTOM_DEVICE_QUERY:
	ld	a,RESULT_NOT_IMPLEMENTED
	ret


;--- Read or write logical sectors from/to a device
;
;    Input:    Cy=0 to read, 1 to write
;              A = Device number, 1 to 255
;              B = Number of sectors to read or write
;              C = Media descriptor byte from the DPB if the device
;                  is a floppy disk drive, zero otherwise
;              HL = Source or destination memory address for the transfer
;              DE = Address where the 4 byte sector number is stored.
;    Output:   A = Error code (the same codes of MSX-DOS are used):
;                  0: Ok
;                  .IDEVN: Invalid device or LUN
;                  .NRDY: Not ready
;                  .DISK: General unknown disk error
;                  .DATA: CRC error when reading
;                  .RNF: Sector not found
;                  .UFORM: Unformatted disk
;                  .WPROT: Write protected media, or read-only logical unit
;                  .WRERR: Write error
;                  .NCOMP: Incompatible disk.
;                  .SEEK: Seek error.
;               B = Sectors successfully transferred

READ_WRITE:
	jp	c,WRSECT
RDSECT:

	or	a				; Check device index
	jp	z,RW_ERR1
	cp	2
	jp	nc,RW_ERR1

	call	MY_GWORK

	call	SDC_ON
	call	WAIT_CMD_RDY
	ld	a,(SDC_STATUS)
	and	SDC_BUSY
	jp	nz,RW_BUSY

	ld	a,(de)
	inc	de
	ld	(ix+4),a
	ld	a,(de)
	inc	de
	ld	(ix+5),a
	ld	a,(de)
	inc	de
	ld	(ix+6),a
	ld	a,(de)
	inc	de
	ld	(ix+7),a			; current sector

	ex	de,hl				; de = destination
	push	bc				; nr of sectors

RD_LOOP:
	push	bc				; nr of sectors
	ld	b,0
	ld	a,(ix+4)
	ld	(SDC_SADDR+0),a
	add	1				; increment sector
	ld	(ix+4),a			; store back incremented
	ld	a,(ix+5)
	ld	(SDC_SADDR+1),a
	adc	b
	ld	(ix+5),a
	ld	a,(ix+6)
	ld	(SDC_SADDR+2),a
	adc	b
	ld	(ix+6),a
	ld	a,(ix+7)
	ld	(SDC_SADDR+3),a
	adc	b
	ld	(ix+7),a
	ld	a,SDC_READ
	ld	(SDC_CMD),a
	call	WAIT_CMD_RDY
	jr	c,R_ERR_LOOP
	ld	bc,512
	ld	hl,SDC_SDATA			; src data
	ldir
	pop	bc
	djnz	RD_LOOP
	pop	bc				; nr of sectors
	call	SDC_OFF

	ld	a,0
	or	a
	ret


R_ERR_LOOP:
	ld	a,(SDC_STATUS)
	ld	e,a				; save stat
	call	SDC_OFF
	pop	bc
	ld	d,b
	pop	bc
	ld	a,d
	sub	b
	ld	b,a				; sectors written
	ld	a,e
	and	SDC_TIMEOUT
	jr	nz,R_TIMEOUT
	ld	a,.DISK
	scf
	ret
R_TIMEOUT:
	ld	a,.NRDY
	scf
	ret

W_ERR_LOOP:
	ld	a,(SDC_STATUS)
	ld	e,a				; save stat
	call	SDC_OFF
	pop	bc
	ld	d,b
	pop	bc
	ld	a,d
	sub	b
	ld	b,a				; sectors written
	ld	a,e
	and	SDC_CRC
	jr	nz,W_WRERR
	ld	a,.DISK
	scf
	ret
W_WRERR:

;	ld
;	call

	ld	a,.WRERR
	scf
	ret

RW_DISK:

;	ld
;	call

	ld	b,.DISK
	jr	RW_ERR
RW_BUSY:

;	ld
;	call

	ld	b,.NRDY
	jr	RW_ERR
RW_ERR1:
	ld  b,.IDEVN
	jr  RW_ERR
;	ld
;	call

	ld	b,.WRERR
RW_ERR:

;	ld
;	call

	call	SDC_OFF
	ld	a,b
	ld	b,0
	scf
	ret

WRSECT:
	or	a				; Check device index
	jr	z,RW_ERR1
	cp	2
	jr	nc,RW_ERR1

	call	MY_GWORK

	call	SDC_ON
	call	WAIT_CMD_RDY
	ld	a,(SDC_STATUS)
	and	SDC_BUSY
	jr	nz,RW_BUSY

	ld	a,(de)
	inc	de
	ld	(ix+4),a
	ld	a,(de)
	inc	de
	ld	(ix+5),a
	ld	a,(de)
	inc	de
	ld	(ix+6),a
	ld	a,(de)
	inc	de
	ld	(ix+7),a			; current sector

	push	bc				; nr of sectors

WR_LOOP:
	push	bc				; nr of sectors
	ld	b,0
	ld	a,(ix+4)
	ld	(SDC_SADDR+0),a
	add	1				; increment sector
	ld	(ix+4),a
	ld	a,(ix+5)
	ld	(SDC_SADDR+1),a
	adc	b
	ld	(ix+5),a
	ld	a,(ix+6)
	ld	(SDC_SADDR+2),a
	adc	b
	ld	(ix+6),a
	ld	a,(ix+7)
	ld	(SDC_SADDR+3),a
	adc	b
	ld	(ix+7),a

	ld	bc,512
	ld	de,SDC_SDATA			; hl = src data
	ldir

	ld	a,SDC_WRITE
	ld	(SDC_CMD),a
	call	WAIT_CMD_RDY
	jp	c,W_ERR_LOOP

	pop	bc
	djnz	WR_LOOP
	pop	bc				; nr of sectors
	call	SDC_OFF

	ld	a,0
	or	a
	ret


;=======================
; Subroutines
;=======================

BINTOHEX:
	ld	c,a
	srl	a
	srl	a
	srl	a
	srl	a
	call	HEXNIBLE
	ld	b,a
	ld	a,c
	and	00fh
	call	HEXNIBLE
	ld	c,a
	ret
HEXNIBLE:
	cp	10
	jr	c,ISDIGT
	add	'A'-10
	ret
ISDIGT:
	add	'0'
	ret

;-----------------------------------------------------------------------------
;
; Enable or disable the SPI registers

SDC_ON:
	ld	a,1
	ld	(SDC_ENABLE),a
	ret

SDC_OFF:
	xor	a
	ld	(SDC_ENABLE),a
	ret

;-----------------------------------------------------------------------------
;
; Wait the BSY flag to clear and RDY flag to be set
; if we wait for more than 30s, send a soft reset to IDE BUS
; if the soft reset didn't work after 30s return with error
;
; Input:  Nothing
; Output: Cy=1 if timeout after soft reset
; Preserves: DE and BC

WAIT_CMD_RDY:
	push	de
	push	bc
	ld	de,2047				; 8142		;Limit the wait to 30s
WAIT_RDY1:
	ld	b,255
WAIT_RDY2:
	ld	a,(SDC_STATUS)
	and	SDC_BUSY+SDC_TIMEOUT+SDC_CRC
	or	a
	jr	z,WAIT_RDY_END
	djnz	WAIT_RDY2
	dec	de
	ld	a,d
	or	e
	jr	nz,WAIT_RDY1
	scf
WAIT_RDY_END:
	pop	bc
	pop	de
	ret

;-----------------------------------------------------------------------------
;
; Read the keyboard matrix to see if ESC is pressed
; Output: Cy = 1 if pressed, 0 otherwise

CHECK_ESC:
	ld	b,7
	in	a,(0AAh)
	and	11110000b
	or	b
	out	(0AAh),a
	in	a,(0A9h)
	bit	2,a
	jr	nz,CHECK_ESC_END
	scf
CHECK_ESC_END:
	ret




;-----------------------------------------------------------------------------
;
; Print a zero-terminated string on screen
; Input: DE = String address

PRINT:
	ld	a,(de)
	or	a
	ret	z
	call	CHPUT
	inc	de
	jr	PRINT

;-----------------------------------------------------------------------------
;
; Obtain the work area address for the driver
; Input: A=1  to obtain the work area for the master, 2 for the slave
; Preserves A

MY_GWORK:
	xor	a
	ex	af,af'
	xor	a
	ld	ix,GWORK
	call	CALBNK
	ret

;SETBORDER:
;	out	(099h),a
;	ld	a,087h
;	out	(099h),a
;	ret

;=======================
; Strings
;=======================

INFO_S:
	db	13,10,"WonderTANG! SMS v"
	db	VER_MAIN+"0",".",VER_SEC+"0",".",VER_REV+"0",13,10
	db	"New juice for your MSX",13,10
	db	"2026 Luis Antoniosi",13,10
	db	"Beautiful British Columbia",13,10
	db	"Canada",13,10
	db	"SS0: Nextor BIOS + MicroSD",13,10
	db	"SS1: FM ROM + OPLL + OPM",13,10
	db	"SS2: Super MegaRAM SCC 2MB",13,10
	db	"SS3: Memory Mapper 4MB",13,10
	db	"---: SMS VDP 32KB ",13,10,0

SEARCH_S:
	db	"Searching: ",0

NODEVS_S:
	db	"Not found",13,10,0

STR_UNKNOWN:
	db	"UNKNOWN",0
SDV1:
	db	"SDV1",0
SDV2:
	db	"SDV2",0
SDHCV2:
	db	"SDHCV2",0
STR_DEVICE_NAME:
	db	"WonderTANG!",0

CRLF_S:
	db	13,10,0


	include	../../sdk/asm/code/output_string.asm

; Pad up to the bank switching code area at 7FD0h; this also makes
; the assembly fail if the driver outgrows the bank.
	ds	7FD0h-$,0FFh

	end
