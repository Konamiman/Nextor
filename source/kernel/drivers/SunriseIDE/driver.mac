	; Driver for the sunrise IDE interface for Nextor
	;
	; Version Beta 5 by Piter Punk
	; Based on version 0.1 by Konamiman

	INCLUDE	../../macros.inc
	INCLUDE	../../const.inc

QUERY_OK: equ 0
QUERY_TRUNCATED_STRING: equ 1
QUERY_INVALID_DEVICE: equ 2
QUERY_INIT_ERROR: equ 3
QUERY_NOT_IMPLEMENTED: equ 0FFh

DRVQ_GET_VERSION: equ 1
DRVQ_GET_STRING: equ 2
DRVQ_GET_INIT_PARAMS: equ 3
DRVQ_INIT: equ 4
DRVQ_GET_MAX_DEVICE: equ 5

DEVQ_GET_STRING: equ 1
DEVQ_GET_PARAMS: equ 2
DEVQ_GET_STATUS: equ 3

	org 4100h

DRV_START:

	ifdef MASTER_ONLY
	.print1 Sunrise IDE - MASTER ONLY driver
	else
	.print1 Sunrise IDE - MASTER AND SLAVE driver
	endif

TESTADD	equ	0F3F5h

;-----------------------------------------------------------------------------
;
; Driver configuration constants
;

;Driver version

VER_MAIN	equ	0
VER_SEC		equ	1
VER_REV		equ	5

;This is a very barebones driver. It has important limitations:
;- CHS mode not supported, disks must support LBA mode.
;- 48 bit addresses are not supported
;  (do the Sunrise IDE hardware support them anyway?)
;- ATAPI devices not supported, only ATA disks.


;-----------------------------------------------------------------------------
;
; IDE registers and bit definitions

IDE_BANK	equ	4104h	;bit 0: enable (1) or disable (0) IDE registers
				;bits 5-7: select 16K ROM bank
IDE_DATA	equ	7C00h	;Data registers, this is a 512 byte area
IDE_ERROR	equ	7E01h	;Error register
IDE_FEAT	equ	7E01h	;Feature register
IDE_SECCNT	equ	7E02h	;Sector count
IDE_SECNUM	equ	7E03h	;Sector number (CHS mode)
IDE_LBALOW	equ	7E03h	;Logical sector low (LBA mode)
IDE_CYLOW	equ	7E04h	;Cylinder low (CHS mode)
IDE_LBAMID	equ	7E04h	;Logical sector mid (LBA mode)
IDE_CYHIGH	equ	7E05h	;Cylinder high (CHS mode)
IDE_LBAHIGH	equ	7E05h	;Logical sector high (LBA mode)
IDE_HEAD	equ	7E06h	;bits 0-3: Head (CHS mode), logical sector higher (LBA mode)
IDE_STATUS	equ	7E07h	;Status register
IDE_CMD		equ	7E07h	;Command register
IDE_DEVCTRL	equ	7E0Eh	;Device control register

; Bits in the error register

UNC	equ	6	;Uncorrectable Data Error
WP	equ	6	;Write protected
MC	equ	5	;Media Changed
IDNF	equ	4	;ID Not Found
MCR	equ	3	;Media Change Requested
ABRT	equ	2	;Aborted Command
NM	equ	1	;No media

M_ABRT	equ	(1 SHL ABRT)

; Bits in the head register

DEV	equ	4	;Device select: 0=master, 1=slave
LBA	equ	6	;0=use CHS mode, 1=use LBA mode

M_DEV	equ	(1 SHL DEV)
M_LBA	equ	(1 SHL LBA)

; Bits in the status register

BSY	equ	7	;Busy
DRDY	equ	6	;Device ready
DF	equ	5	;Device fault
DRQ	equ	3	;Data request
ERR	equ	0	;Error

M_BSY	equ	(1 SHL BSY)
M_DRDY	equ	(1 SHL DRDY)
M_DF	equ	(1 SHL DF)
M_DRQ	equ	(1 SHL DRQ)
M_ERR	equ	(1 SHL ERR)

; Bits in the device control register register

SRST	equ	2	;Software reset

M_SRST	equ	(1 SHL SRST)


;-----------------------------------------------------------------------------
;
; Standard BIOS and work area entries

;CHPUT	equ	00A2h	;Character output
CHGET	equ	009Fh


;-----------------------------------------------------------------------------
;
; Work area definition
;
;+0: Device and logical units types for master device
;    bits 0,1: Device type
;              00: No device connected
;              01: ATA hard disk, CHS only
;              10: ATA hard disk, LBA supported
;              11: ATAPI device
;    bits 2,3: Device type for LUN 1 on master device
;              00: Block device
;              01: Other, non removable
;              10: CD-ROM
;              11: Other, removable
;    bits 4,5: Device type for LUN 2 on master device
;    bits 6,7: Device type for LUN 3 on master device
;
;+1: Logical unit types for master device
;    bits 0,1: Device type for LUN 4 on master device
;    bits 2,3: Device type for LUN 5 on master device
;    bits 4,5: Device type for LUN 6 on master device
;    bits 6,7: Device type for LUN 7 on master device
;
;+2,3: Reserved for CHS data for the master device (to be implemented)
;
;+4..+7: Same as +0..+3, for the slave device
;
; Note: Actually, due to driver limitations, currently only the
; "device type" bits are used, and with possible values 00 and 10 only.
; LUN type bits are always 00.


;-----------------------------------------------------------------------------
;
; Error codes for DEV_RW and DEV_FORMAT
;

.NCOMP	equ	0FFh
.WRERR	equ	0FEh
.DISK	equ	0FDh
.NRDY	equ	0FCh
.DATA	equ	0FAh
.RNF	equ	0F9h
.WPROT	equ	0F8h
.UFORM	equ	0F7h
.SEEK	equ	0F3h
.IFORM	equ	0F0h
.IDEVN	equ	0B5h
.IPARM	equ	08Bh

;-----------------------------------------------------------------------------
;
; Routines available on kernel page 0
;

;* Get in A the current slot for page 1. Corrupts F.
;  Must be called by using CALBNK to bank 0:
;  xor a
;  ld ix,GSLOT1
;  call CALBNK

GSLOT1	equ	402Dh


;* This routine reads a byte from another bank.
;  Must be called by using CALBNK to the desired bank,
;  passing the address to be read in HL:
;  ld a,bank
;  ld hl,address
;  ld ix,RDBANK
;  call CALBNK

RDBANK	equ	403Ch


;* This routine temporarily switches kernel bank 0/3,
;  then jumps to CALBAS in MSX BIOS.
;  This is necessary so that kernel bank is correct in case of BASIC error.

CALBAS	equ	403Fh


;* Call a routine in another bank.
;  Must be used if the driver spawns across more than one bank.
;  Input: A = bank
;         IX = routine address
;         AF' = AF for the routine
;         BC, DE, HL, IY = input for the routine

CALBNK	equ	4042h


;* Get in IX the address of the SLTWRK entry for the slot passed in A,
;  which will in turn contain a pointer to the allocated page 3
;  work area for that slot (0 if no work area was allocated).
;  If A=0, then it uses the slot currently switched in page 1.
;  Returns A=current slot for page 1, if A=0 was passed.
;  Corrupts F.
;  Must be called by using CALBNK to bank 0:
;  ld a,slot
;  ex af,af'
;  xor a
;  ld ix,GWORK
;  call CALBNK

GWORK	equ	4045h


;* Call a routine in the driver bank.
;  Input: (BK4_ADD) = routine address
;         AF, BC, DE, HL, IY = input for the routine
;
; Calls a routine in the driver bank. This routine is the same as CALBNK,
; except that the routine address is passed in address BK4_ADD (#F2ED)
; instead of IX, and the bank number is always 5. This is useful when used
; in combination with CALSLT to call a driver routine from outside
; the driver itself.
;
; Note that register IX can't be used as input parameter, it is
; corrupted before reaching the invoked code.

CALDRV	equ	4048h


;-----------------------------------------------------------------------------
;
; Built-in format choice strings
;

NULL_MSG  equ     741Fh	;Null string (disk can't be formatted)
SING_DBL  equ     7420h ;"1-Single side / 2-Double side"


;-----------------------------------------------------------------------------
;
	;Driver signature

	db	"NEXTORv3_DRIVER",0

	;Jump table

	jp	DRV_TIMI ;TIMER_INT
	jp	DRV_BASSTAT ;OEMSTAT
	jp	DRV_BASDEV ;BASDEV
	jp	DRV_EXTBIO ;EXTBIO
	jp	DRV_DIRECT0 ;DIRECT_0
	jp	DRV_DIRECT1 ;DIRECT_1
	jp	DRV_DIRECT2 ;DIRECT_2
	jp	DRV_DIRECT3 ;DIRECT_3
	jp	DRV_DIRECT4 ;DIRECT_4
	jp	DRIVER_QUERY
	jp	DEVICE_QUERY
	jp	CUSTOM_DRIVER_QUERY
	jp  CUSTOM_DEVICE_QUERY
	jp	READ_WRITE


DRV_NAME:
	db	"Sunrise IDE",0


;-----------------------------------------------------------------------------
;
; Compatibility layer for translating Nextor v2 driver routines
; to the Nextor v3 driver structure


	;Output a string
	;Input:  HL = String
	;        DE = Destination
	;        B  = Max length including terminator
	;Output: A  = QUERY_OK or QUERY_TRUNCATED_STRING 
	
OUTPUT_STRING:
	ld a,b
	or a
	ret z

OUTPUT_STRING_LOOP:
	ld a,(hl)
	or a
	ld (de),a
	ret z

	inc hl
	inc de
	djnz OUTPUT_STRING

    dec de
	xor a
	ld (de),a
	ld a,QUERY_TRUNCATED_STRING
	ret


	;--- Driver query
	;    Input:  A = Query index
	;            F, BC, DE, HL = Depends on the query
	;    Output: A = Error code:
	;                QUERY_OK: success
	;                QUERY_NOT_IMPLEMENTED: query not implemented
	;                Others: depends on the query
	;            F, BC, DE, HL = Depends on the query

DRIVER_QUERY:
	dec a
	jr z,DO_DRVQ_GET_VERSION
	dec a
	jr z,DO_DRVQ_GET_STRING
	dec a
	jr z,DO_DRVQ_GET_INIT_PARAMS
	dec a
	jr z,DO_DRVQ_INIT
	dec a
	jr z,DO_DRVQ_GET_MAX_DEVICE
	ld a,QUERY_NOT_IMPLEMENTED
	ret

DO_DRVQ_GET_VERSION:
	call NEXTOR2_DRV_VERSION
	ld d,c
	ld c,b
	ld b,a
	xor a
	ret

DO_DRVQ_GET_STRING:
	ld a,b	;String index
	ld b,d	;Buffer size
	ex de,hl
	dec a
	ld hl,DRV_NAME
	jp z,OUTPUT_STRING
	ld a,QUERY_NOT_IMPLEMENTED
	ret

DO_DRVQ_GET_INIT_PARAMS:
	push de
	pop iy
	xor a
	call NEXTOR2_DRV_INIT
	ld b,0
	rl b
	xor a
	ret

DO_DRVQ_INIT:
	push de
	pop iy
	ld a,1
	call NEXTOR2_DRV_INIT
	xor a
	ret

DO_DRVQ_GET_MAX_DEVICE:
	ifdef MASTER_ONLY
	ld b,1
	else
	ld b,2
	endif

	xor a
	ret

CHPUT: jp (iy)


	;--- Device query
	;    Input:  A = Query index
	;            C = Device number
	;            F, B, DE, HL = Depends on the query
	;    Output: A = Error code:
	;                QUERY_OK: success
	;                QUERY_INVALID_DEVICE: Invalid device number
	;                QUERY_NOT_IMPLEMENTED: query not implemented
	;                Others: depends on the query
	;            F, BC, DE, HL = Depends on the query

DEVICE_QUERY:
	push af
	ld a,c
	or a
	jr z,INVALID_DEVICE

	ifdef MASTER_ONLY
	dec a
	jr nz,INVALID_DEVICE
	else
	cp 3
	jr nc,INVALID_DEVICE
	endif

	pop af
	dec a
	jr z,DO_DEVQ_GET_STRING
	dec a
	jr z,DO_DEVQ_GET_PARAMS
	dec a
	jr z,DO_DEVQ_GET_STATUS
	dec a
	jr z,DO_DEVQ_GET_AVAILABILITY
	ld a,QUERY_NOT_IMPLEMENTED
	ret

INVALID_DEVICE:
	pop af
	ld a,QUERY_INVALID_DEVICE
	ret

DO_DEVQ_GET_STRING:
	ld a,b
	or a
	jr z,RETURN_NOT_IMP

	ld a,d
	or a
	ret z	   ;Buffer size=0: do nothing, no errorr
	dec a
	jr nz,DO_DEVQ_GET_STRING_2
	ld (hl),0  ;Buffer size=0: just output terminating 0, no error
	ret
DO_DEVQ_GET_STRING_2:

	ld a,c	;Device number
	push de
	call NEXTOR2_DEV_INFO
	pop de
	or a
	jr nz,RETURN_NOT_IMP	;Assume no "invalid device" error (we checked device id first)

	;IDE strings are 20 char long, so if buffer was at least 21 bytes long
	;assume success, otherwise assume string was truncated
	ld a,d
	cp 21
	ld a,0
	ret nc
	ld a,QUERY_TRUNCATED_STRING
	ret

DO_DEVQ_GET_PARAMS:
	ld a,h
	or l
	ret z	;No buffer: just return no error (device id is ok)

	ld a,c
	ld b,1
	push hl
	call NEXTOR2_LUN_INFO
	pop ix
	or a
	ret z

	;Assume error is "device not available" (we checked the device id first),
	;then return default parameters but with removable bit set
	xor a
	ld (ix),a
	ld (ix+1),a
	ld (ix+2),2	;Sector size, high byte
	ld (ix+3),a
	ld (ix+4),a
	ld (ix+5),a
	ld (ix+6),a
	ld (ix+7),1	;Removable flag
	ld (ix+8),a
	ld (ix+9),a
	ld (ix+10),a
	ld (ix+11),a
	ret

DO_DEVQ_GET_STATUS:
DO_DEVQ_GET_AVAILABILITY:
	ld a,c
	ld b,1
	call NEXTOR2_DEV_STATUS
	ld b,a
	;Assume A=0 means "device not available" and not "invalid device id"
	;(we checked the device id first)
	xor a
	ret

CUSTOM_DRIVER_QUERY:
CUSTOM_DEVICE_QUERY:
	ld a,QUERY_NOT_IMPLEMENTED
	ret

READ_WRITE:
	ld c,1
	call NEXTOR2_DEV_RW
	ld b,c
	ret

RETURN_NOT_IMP:
	ld a,QUERY_NOT_IMPLEMENTED
	ret

;-----------------------------------------------------------------------------
;
; Timer interrupt routine, it will be called on each timer interrupt
; (at 50 or 60Hz), but only if DRV_INIT returns Cy=1 on its first execution.

DRV_TIMI:
	ret


;-----------------------------------------------------------------------------
;
; Driver initialization, it is called twice:
;
; 1) First execution, for information gathering.
;    Input:
;      A = 0
;      B = number of available drives (drive-based drivers only)
;      HL = maximum size of allocatable work area in page 3
;    Output:
;      A = number of required drives (for drive-based driver only)
;      HL = size of required work area in page 3
;      Cy = 1 if DRV_TIMI must be hooked to the timer interrupt, 0 otherwise
;
; 2) Second execution, for work area and hardware initialization.
;    Input:
;      A = 1
;      B = number of allocated drives for this controller
;          (255 if device-based driver, unless 4 is pressed at boot)
;
;    The work area address can be obtained by using GWORK.
;
;    If first execution requests more work area than available,
;    second execution will not be done and DRV_TIMI will not be hooked
;    to the timer interrupt.
;
;    If first execution requests more drives than available,
;    as many drives as possible will be allocated, and the initialization
;    procedure will continue the normal way
;    (for drive-based drivers only. Device-based drivers always
;     get two allocated drives.)

TEMP_WORK	equ	0C000h

NEXTOR2_DRV_INIT:
	;--- If first execution, just inform that no work area is needed
	;    (the 8 bytes in SLTWRK are enough)

	or	a
	ld	hl,0
	ld	a,2
	ret	z			;Note that Cy is 0 (no interrupt hooking needed)

	ld	de,INFO_S
	call	PRINT

	ld	de,SEARCH_S
	call	PRINT

	ld	a,1
	call	MY_GWORK
	call	IDE_ON
	ld	(ix),0			;Assume both devices empty
	ld	(ix+4),0	

        ld      a,M_SRST		;Do a software reset
        ld      (IDE_DEVCTRL),a
        nop     ;Wait 5 us
        xor     a
        ld      (IDE_DEVCTRL),a

WAIT_RESET:
        ld      de,7640			;Timeout after 30 s
WAIT_RESET1:
        ld      a,0
        cp      e
        jr      nz,WAIT_DOT		;Print dots while waiting
        ld      a,46
        call    CHPUT
WAIT_DOT:
	call	CHECK_ESC
	jp	c,INIT_NO_DEV
        ld      b,255
WAIT_RESET2:
        ld      a,(IDE_STATUS)
        and     M_BSY+M_DRDY
        cp      M_DRDY
        jr      z,WAIT_RESET_END        ;Wait for BSY to clear and DRDY to set          
        djnz    WAIT_RESET2
        dec     de
        ld      a,d
        or      e
        jr      nz,WAIT_RESET1
        jp      INIT_NO_DEV
WAIT_RESET_END:

	;--- Do a quick pre-check on MASTER device

	ld	a,0
	ld	(IDE_HEAD),a		;Select device 0
	nop

	call	INIT_PRECHECK_DEV	
	ld	a,(IDE_SECCNT)
	cp	85
	jr	nz,MASTER_CHECK1_END
	ld	a,(IDE_SECNUM)
	cp	170
	jr	nz,MASTER_CHECK1_END

	ld	a,1			;Flag the device
	ld	(ix),a
MASTER_CHECK1_END:

ifndef MASTER_ONLY

        ld      a,46			;Print dot
        call    CHPUT
	
	;--- Same pre-check on SLAVE device

	call	WAIT_CMD_RDY	
	jr	c,SLAVE_CHECK1_END
	ld	a,M_DEV
	ld	(IDE_HEAD),a		;Select device 1

	call	INIT_PRECHECK_DEV	
	ld	a,(IDE_SECCNT)
	cp	85
	jr	nz,SLAVE_CHECK1_END
	ld	a,(IDE_SECNUM)
	cp	170
	jr	nz,SLAVE_CHECK1_END

	ld	a,1			;Flag the device
	ld	(ix+4),a
SLAVE_CHECK1_END:
        ld      a,46			;Print dot
        call    CHPUT
       
        ld      a,M_SRST		; Do ANOTHER software reset
        ld      (IDE_DEVCTRL),a
        nop     			;Wait 5 us
        xor     a
        ld      (IDE_DEVCTRL),a
	nop				;Wait 5 us
        ld      a,46			;Print dot
        call    CHPUT

	ld      de,CRLF_S
        call    PRINT

endif

	;--- Get info and show the name for the MASTER

	ld	de,MASTER_S
	call	PRINT

WSKIPMAS:			; If ESC is pressed, ignore this device
        ld      de,624			; Wait 1s to read the keyboard
WSKIPMAS1:
        call    CHECK_ESC
        jr      c,NODEV_MASTER
        ld      b,64
WSKIPMAS2:
	ex	(sp),hl
	ex	(sp),hl
        djnz    WSKIPMAS2
        dec     de
        ld      a,d
        or      e
        jr      nz,WSKIPMAS1

	ld	a,(ix)			;If the device isn't flagged it doesn't exists
	cp	1
	jr	nz,NODEV_MASTER
        ld      a,46			;Print FIRST dot
        call    CHPUT

	call	WAIT_CMD_RDY
	jr	c,NODEV_MASTER
	ld	a,0
	ld	(IDE_HEAD),a		;Select device 0
        ld      a,46			;Print SECOND dot
        call    CHPUT

	ld	a,0ECh			;Send IDENTIFY commad
	call	DO_IDE			
	jr	c,NODEV_MASTER
        ld      a,46			;Print THIRD dot
        call    CHPUT

	call	INIT_CHECK_DEV		;Check if the device is ATA or ATAPI
	jr	c,NODEV_MASTER
        ld      a,46			;Print FOURTH dot
        call    CHPUT

	call	WAIT_CMD_RDY		;Try to select the device
	jr	c,NODEV_MASTER		;this is our last chance to *NOT* detect it
	ld	a,0
	ld	(IDE_HEAD),a		;Select device 0
        ld      a,46			;Print FIFTH dot
        call    CHPUT

	call	INIT_PRINT_NAME

	ld	(ix),2	;ATA device with LBA
	jr	OK_MASTER

NODEV_MASTER:
	call	CHECK_ESC
	jr	c,NODEV_MASTER

	ld	(ix),0	
	ld	de,NODEVS_S
	call	PRINT
	
OK_MASTER:

ifndef MASTER_ONLY

	;--- Get info and show the name for the SLAVE
	
	ld	de,SLAVE_S
	call	PRINT

WAIT_SKIP_SLAVE:			; If ESC is pressed, ignore this device
        ld      de,624			; Wait 1s to read the keyboard
WAIT_SKIP_SLAVE1:
        call    CHECK_ESC
        jr      c,NODEV_SLAVE
        ld      b,64
WAIT_SKIP_SLAVE2:
	ex	(sp),hl
	ex	(sp),hl
        djnz    WAIT_SKIP_SLAVE2
        dec     de
        ld      a,d
        or      e
        jr      nz,WAIT_SKIP_SLAVE1

	ld	a,(ix+4)		;If the device isn't flagged it doesn't exists
	cp	1
	jr	nz,NODEV_SLAVE
        ld      a,46			;Print FIRST dot
        call    CHPUT

	call	WAIT_CMD_RDY	
	jr	c,NODEV_SLAVE
	ld	a,M_DEV
	ld	(IDE_HEAD),a		;Select device 1
        ld      a,46			;Print SECOND dot
        call    CHPUT
	
	ld	a,0ECh
	call	DO_IDE
	jr	c,NODEV_SLAVE		;If error, no device, or ATAPI device
        ld      a,46			;Print THIRD dot
        call    CHPUT

	call	INIT_CHECK_DEV
	jr	c,NODEV_SLAVE
        ld      a,46			;Print FOURTH dot
        call    CHPUT

	call	WAIT_CMD_RDY	
	jr	c,NODEV_SLAVE
	ld	a,M_DEV
	ld	(IDE_HEAD),a		;Select device 1
        ld      a,46			;Print FIFTH dot
        call    CHPUT
	
	call	INIT_PRINT_NAME

	ld	(ix+4),2		;ATA device with LBA
	jr	OK_SLAVE

NODEV_SLAVE:
	call	CHECK_ESC
	jr	c,NODEV_SLAVE

	ld	(ix+4),0
	ld	de,NODEVS_S
	call	PRINT
OK_SLAVE:

        ld      a,M_SRST		;Last software reset before we go
        ld      (IDE_DEVCTRL),a		;some times a faulty slave leaves
					;BSY set forever (30s)
        nop     ;Wait 5 us
        xor     a
        ld      (IDE_DEVCTRL),a

endif

	jr	DRV_INIT_END

INIT_NO_DEV:
	call	CHECK_ESC
	jr	c,INIT_NO_DEV

	ld      de,CRLF_S
        call    PRINT
	ld	de,MASTER_S
	call	PRINT
	ld	de,NODEVS_S
	call	PRINT

ifndef MASTER_ONLY

	ld	de,SLAVE_S
	call	PRINT

endif

	ld	de,NODEVS_S
	call	PRINT
	
	;--- End of the initialization procedure

DRV_INIT_END:
	call	IDE_OFF
	ret

;--- Subroutines for the INIT procedure

;Check if there is any device listening in the bus
;Input: device already selected
;Output: If something is there, IDE_SECCNT=85, IDE_SECNUM=170
;	 Both variables have random values if nothing is there

INIT_PRECHECK_DEV:
	ld	a,85
	ld	(IDE_SECCNT),a
	ld	a,170
	ld	(IDE_SECNUM),a
	ld	a,170
	ld	(IDE_SECCNT),a
	ld	a,85
	ld	(IDE_SECNUM),a
	ld	a,85
	ld	(IDE_SECCNT),a
	ld	a,170
	ld	(IDE_SECNUM),a
	ret

;Check that a device is present and usable.
;Input:  IDENTIFY DEVICE issued successfully.
;Output: Cy=0 for device ok, 1 for no device or not usable.
;        If device ok, 50 first bytes of IDENTIFY device copied to TEMP_WORK.

INIT_CHECK_DEV:
	ld	hl,IDE_DATA
	ld	de,TEMP_WORK
	ld	bc,50*2	;Get the first 50 data words
	ldir

	ld	a,(IDE_STATUS)		;Check status
	cp	01111111b		;Usually this means "no device"
	jr	z,INIT_CHECK_NODEV

	; "At power-up or after reset, the Command Block Registers are initialized 
	;  to the following values:
	;
	; 	REGISTER          VALUE
	; 	1F1 Error         : 01
	; 	1F2 Sector Count  : 01
	; 	1F3 Sector Number : 01
	; 	1F4 Cylinder Low  : 00
	; 	1F5 Cylinder High : 00
	; 	1F6 Drive / Head  : 00"
	;
	; Not all devices respect this. One of my CompactFlash cards never have
	; Sector Count = 01 and Sector Number = 01 after reset.
	;
	;	ld	a,(IDE_SECNUM)		;Test if the device is REALLY here
	;	cp	1
	;	jr	nz,INIT_CHECK_NODEV
	;	ld	a,(IDE_SECCNT)
	;	cp	1
	;	jr	nz,INIT_CHECK_NODEV

	ld	a,(IDE_CYLOW)		;Test for PATAPI devices
	cp	20 
	jr	nz,TEST2_FOR_ATAPI
	ld	a,(IDE_CYHIGH)
	cp	235
	jr	z,INIT_CHECK_NODEV
TEST2_FOR_ATAPI:
	ld	a,(IDE_CYLOW)		;Test for SATAPI devices
	cp	105
	jr	nz,TEST_FOR_ATA
	ld	a,(IDE_CYHIGH)
	cp	150
	jr	z,INIT_CHECK_NODEV

TEST_FOR_ATA:
	ld	a,(IDE_CYLOW)		;Test for PATA devices
	cp	0
	jr	nz,TEST2_FOR_ATA
	ld	a,(IDE_CYHIGH)
	cp	0
	jr	z,TEST_FOR_LBA
TEST2_FOR_ATA:
	ld	a,(IDE_CYLOW)		;Test for SATA devices
	cp	60
	jr	nz,INIT_CHECK_NODEV
	ld	a,(IDE_CYHIGH)
	cp	195
	jr	nz,INIT_CHECK_NODEV
	
TEST_FOR_LBA:
	ld      a,(TEMP_WORK+49*2+1)
	and	2			;LBA supported?
	jr	z,INIT_CHECK_NODEV
	xor	a
	ret

INIT_CHECK_NODEV:
	scf
	ret


;Print a device name.
;Input: 50 first bytes of IDENTIFY device on TEMP_WORK.

INIT_PRINT_NAME:
	ld	hl,TEMP_WORK+27*2
	ld	b,20
DEVNAME_LOOP:
	ld	c,(hl)
	inc	hl
	ld	a,(hl)
	inc	hl
	call	CHPUT
	ld	a,c
	call	CHPUT
	djnz	DEVNAME_LOOP

	ld	de,CRLF_S
	call	PRINT
	ret


;-----------------------------------------------------------------------------
;
; Obtain driver version
;
; Input:  -
; Output: A = Main version number
;         B = Secondary version number
;         C = Revision number

NEXTOR2_DRV_VERSION:
	ld	a,VER_MAIN
	ld	b,VER_SEC
	ld	c,VER_REV
	ret


;-----------------------------------------------------------------------------
;
; BASIC expanded statement ("CALL") handler.
; Works the expected way, except that CALBAS in kernel page 0
; must be called instead of CALBAS in MSX BIOS.

DRV_BASSTAT:
	scf
	ret


;-----------------------------------------------------------------------------
;
; BASIC expanded device handler.
; Works the expected way, except that CALBAS in kernel page 0
; must be called instead of CALBAS in MSX BIOS.

DRV_BASDEV:
	scf
	ret


;-----------------------------------------------------------------------------
;
; Extended BIOS hook.
; Works the expected way, except that it must return
; D'=1 if the old hook must be called, D'=0 otherwise.
; It is entered with D'=1.

DRV_EXTBIO:
	ret


;-----------------------------------------------------------------------------
;
; Direct calls entry points.
; Calls to addresses 7450h, 7453h, 7456h, 7459h and 745Ch
; in kernel banks 0 and 3 will be redirected
; to DIRECT0/1/2/3/4 respectively.
; Receives all register data from the caller except IX and AF'.

DRV_DIRECT0:
DRV_DIRECT1:
DRV_DIRECT2:
DRV_DIRECT3:
DRV_DIRECT4:
	ret


;-----------------------------------------------------------------------------
;
; Read or write logical sectors from/to a logical unit
;
;Input:    Cy=0 to read, 1 to write
;          A = Device number, 1 to 7
;          B = Number of sectors to read or write
;          C = Logical unit number, 1 to 7
;          HL = Source or destination memory address for the transfer
;          DE = Address where the 4 byte sector number is stored
;Output:   A = Error code (the same codes of MSX-DOS are used):
;              0: Ok
;              .IDEVN: Invalid device or LUN
;              .NRDY: Not ready
;              .DISK: General unknown disk error
;              .DATA: CRC error when reading
;              .RNF: Sector not found
;              .UFORM: Unformatted disk
;              .WPROT: Write protected media, or read-only logical unit
;              .WRERR: Write error
;              .NCOMP: Incompatible disk
;              .SEEK: Seek error
;          B = Number of sectors actually read/written

NEXTOR2_DEV_RW:
	push	af

	ld	a,b	;Swap B and C
	ld	b,c
	ld	c,a
	pop	af
	push	af
	push	bc
	call	CHECK_DEV_LUN
	pop	bc
	jp	c,DEV_RW_NODEV

	dec	a
	jr	z,DEV_RW2
	ld	a,M_DEV
DEV_RW2:
	ld	b,a

	ld	a,c
	or	a
	jr	nz,DEV_RW_NO0SEC
	pop	af
	xor	a
	ld	b,0
	ret	
DEV_RW_NO0SEC:

	push	de
	pop	ix
	ld	a,(ix+3)
	and	11110000b
	jp	nz,DEV_RW_NOSEC	;Only 28 bit sector numbers supported

	call	IDE_ON

	ld	a,(ix+3)
	or	M_LBA
	or	b
	ld	(IDE_HEAD),a	;IDE_HEAD must be written first,
	ld	a,(ix)		;or the other IDE_LBAxxx and IDE_SECCNT
	ld	(IDE_LBALOW),a	;registers will not get a correct value
	ld	a,(ix+1)	;(blueMSX issue?)
	ld	(IDE_LBAMID),a
	ld	a,(ix+2)
	ld	(IDE_LBAHIGH),a
	ld	a,c
	ld	(IDE_SECCNT),a
	
	pop	af
	jr	c,DEV_DO_WR

	;---
	;---  READ
	;---

	call	WAIT_CMD_RDY
	jr	c,DEV_RW_ERR
	ld	a,20h
	push	bc	;Save sector count
	call	DO_IDE
	pop	bc
	jr	c,DEV_RW_ERR

	call	DEV_RW_FAULT
	ret	nz

	ld	b,c	;Retrieve sector count
	ex	de,hl
DEV_R_GDATA:
	push	bc
	ld	hl,IDE_DATA
	ld	bc,512
	ldir
	pop	bc
	djnz	DEV_R_GDATA

	call	IDE_OFF
	xor	a
	ret
	
	;---
	;---  WRITE
	;---

DEV_DO_WR:
	call	WAIT_CMD_RDY
	jr	c,DEV_RW_ERR
	ld	a,30h
	push	bc	;Save sector count
	call	DO_IDE
	pop	bc
	jr	c,DEV_RW_ERR

	ld	b,c	;Retrieve sector count
DEV_W_LOOP:
	push	bc
	ld	de,IDE_DATA
	ld	bc,512
	ldir
	pop	bc

	call	WAIT_IDE
	jr	c,DEV_RW_ERR

	call	DEV_RW_FAULT
	ret	nz

	djnz	DEV_W_LOOP

	call	IDE_OFF
	xor	a
	ret

	;---
	;---  ERROR ON READ/WRITE
	;---

DEV_RW_ERR:
	ld	a,(IDE_ERROR)
	ld	b,a
	call	IDE_OFF
	ld	a,b	

	bit	NM,a	;Not ready
	jr	nz,DEV_R_ERR1
	ld	a,.NRDY
	ld	b,0
	ret
DEV_R_ERR1:

	bit	IDNF,a	;Sector not found
	jr	nz,DEV_R_ERR2
	ld	a,.RNF
	ld	b,0
	ret
DEV_R_ERR2:

	bit	WP,a	;Write protected
	jr	nz,DEV_R_ERR3
	ld	a,.WPROT
	ld	b,0
	ret
DEV_R_ERR3:

	ld	a,.DISK	;Other error
	ld	b,0
	ret

	;--- Check for device fault
	;    Output: NZ and A=.DISK on fault

DEV_RW_FAULT:
	ld	a,(IDE_STATUS)
	and	M_DF	;Device fault
	ret	z

	call	IDE_OFF
	ld	a,.DISK
	ld	b,0
	or	a
	ret

	;--- Termination points

DEV_RW_NOSEC:
	call	IDE_OFF
	pop	af
	ld	a,.RNF
	ld	b,0
	ret

DEV_RW_NODEV:
	call	IDE_OFF
	pop	af
	ld	a,.IDEVN
	ld	b,0
	ret


;-----------------------------------------------------------------------------
;
; Device information gathering
;
;Input:   A = Device number, 1 to 7
;         B = Information to return:
;             0: Basic information
;             1: Manufacturer name string
;             2: Device name string
;             3: Serial number string
;         HL = Pointer to a buffer in RAM
;         D  = Buffer length (added in Nextor 3)
;Output:  A = Error code:
;             0: Ok
;             1: Device not available or invalid device number
;             2: Information not available, or invalid information index
;         When basic information is requested,
;         buffer filled with the following information:
;
;+0 (1): Numer of logical units, from 1 to 8. 1 if the device has no logical
;        drives (which is functionally equivalent to having only one).
;+1 (1): Flags, always zero
;
; The strings must be printable ASCII string (ASCII codes 32 to 126),
; left justified and padded with spaces. All the strings are optional,
; if not available, an error must be returned.
; If a string is provided by the device in binary format, it must be reported
; as an hexadecimal, upper-cased string, preceded by the prefix "0x".
; The maximum length for a string is 64 characters;
; if the string is actually longer, the leftmost 64 characters
; should be provided.
;
; In the case of the serial number string, the same rules for the strings
; apply, except that it must be provided right-justified,
; and if it is too long, the rightmost characters must be
; provided, not the leftmost.

NEXTOR2_DEV_INFO:
	or	a	;Check device number
	jp	z,DEV_INFO_ERR1
	cp	3
	jp	nc,DEV_INFO_ERR1

	push de
	call	MY_GWORK
	pop de

	ld	c,a
	ld	a,b
	or	a
	jr	nz,DEV_INFO_STRING

	;--- Obtain basic information

	ld	a,(ix)
	or	a	;Device available?
	jp	z,DEV_INFO_ERR1

	ld	(hl),1	;One single LUN
	inc	hl
	ld	(hl),0	;Always zero
	xor	a
	ret

	;--- Obtain string information

DEV_INFO_STRING:
	push de		;Save buffer length
	ld a,d

	push	hl
	push	bc
	push	hl
	pop	de
	inc	de
	ld	(hl)," "
	ld	c,a
	ld  b,0
	dec bc
	ldir
	pop	bc
	pop	hl

	call	IDE_ON
	pop de	;Buffer length

	ld	a,c
	dec	a
	jr	z,DEV_INFO_STRING2
	ld	a,M_DEV

DEV_INFO_STRING2:
	ld	c,a	;C=Device flag for the HEAD register
	ld	a,b

	dec	a
	jr	z,DEV_INFO_ERR2	;Manufacturer name

	;--- Device name

	dec	a
	jr	nz,DEV_STRING_NO1

	ld	b,27
	push de
	call	DEV_STING_PREPARE
	pop de
	jr	c,DEV_INFO_ERR1

DEV_STRING_DO:
	ld a,d
	dec a	;Don't count terminating 0
	cp 21
	ld b,a
	jr c,DEV_STRING_SKIP_SPACES
	ld b,20

DEV_STRING_SKIP_SPACES:
	ld	de,(IDE_DATA)
	ld	a,d
	cp 33
	jr nc,DEV_STRING_LOOP_NOSPACE_1
	dec b
	jr z,DEVSTR_END
	ld	a,e
	cp 33
	jr nc,DEV_STRING_LOOP_NOSPACE_2
	dec b
	jr z,DEVSTR_END
	jr DEV_STRING_SKIP_SPACES

DEV_STRING_LOOP:
	ld	de,(IDE_DATA)
	ld	a,d
DEV_STRING_LOOP_NOSPACE_1:
	cp	32
	jr c,DEVSTRLOOP_1_CTRL
	cp	127
	jr	c,DEVSTRLOOP_1
DEVSTRLOOP_1_CTRL:
	ld	a," "
DEVSTRLOOP_1:
	ld	(hl),a
	inc	hl
	dec b
	jr z,DEVSTR_END
	ld	a,e
DEV_STRING_LOOP_NOSPACE_2:
	cp	32
	jr	c,DEVSTRLOOP_2_CTRL
	cp	127
	jr	c,DEVSTRLOOP_2
DEVSTRLOOP_2_CTRL:	
	ld	a," "
DEVSTRLOOP_2:
	ld	(hl),a
	inc	hl
	djnz	DEV_STRING_LOOP
DEVSTR_END:	
	ld (hl),0	;Terminating 0

	call	IDE_OFF
	xor	a
	ret

DEV_STRING_NO1:

	;--- Serial number

	dec	a
	jr	nz,DEV_INFO_ERR2	;Unknown string

	ld	b,10
	push de
	call	DEV_STING_PREPARE
	pop de
	jr	c,DEV_INFO_ERR1

	ld	b,10
	jr	DEV_STRING_DO
	
	;--- Termination with error

DEV_INFO_ERR1:
	call	IDE_OFF
	ld	a,1
	ret

DEV_INFO_ERR2:
	call	IDE_OFF
	ld	a,2
	ret



;Common processing for obtaining a device information string
;Input: B  = Offset of the string in the device information (words)
;       HL = Destination address for the string
;       C  = Device flag for the HEAD register
;       D  = Buffer length
;Corrupts AF, DE

DEV_STING_PREPARE:
	push de
	call	WAIT_CMD_RDY
	ld	a,c		;Issue IDENTIFY DEVICE command
	ld	(IDE_HEAD),a
	ld	a,0ECh
	call	DO_IDE
	pop de
	ret	c

	ld a,d
	push	hl		;Fill destination with spaces
	push	bc
	push	hl
	pop	de
	inc	de
	ld	(hl)," "
	ld	c,a
	ld b,0
	dec bc
	ldir
	pop	bc
	pop	hl

DEV_STRING_SKIP:
	ld	de,(IDE_DATA)	;Skip device data until the desired string
	djnz	DEV_STRING_SKIP

	ret


;-----------------------------------------------------------------------------
;
; Obtain device status
;
;Input:   A = Device number, 1 to 7
;         B = Logical unit number, 1 to 7.
;             0 to return the status of the device itself.
;Output:  A = Status for the specified logical unit,
;             or for the whole device if 0 was specified:
;                0: The device or logical unit is not available, or the
;                   device or logical unit number supplied is invalid.
;                1: The device or logical unit is available and has not
;                   changed since the last status request.
;                2: The device or logical unit is available and has changed
;                   since the last status request
;                   (for devices, the device has been unplugged and a
;                    different device has been plugged which has been
;                    assigned the same device number; for logical units,
;                    the media has been changed).
;                3: The device or logical unit is available, but it is not
;                   possible to determine whether it has been changed
;                   or not since the last status request.
;
; Devices not supporting hot-plugging must always return status value 1.
; Non removable logical units may return values 0 and 1.

NEXTOR2_DEV_STATUS:
	set	0,b	;So that CHECK_DEV_LUN admits B=0

	call	CHECK_DEV_LUN
	ld	e,a
	ld	a,0
	ret	c

	ld	a,1	;Never changed
	ret

	ld	a,e
	cp	2
	ld	a,1
	ret	nz

	ld	a,e
	dec	a	;FOR TESTING:
	ld	a,2	;Return "Unchanged" for device 1, "Unknown" for device 2
	ret	z
	ld	a,3
	ret


;-----------------------------------------------------------------------------
;
; Obtain logical unit information
;
;Input:   A  = Device number, 1 to 7.
;         B  = Logical unit number, 1 to 7.
;         HL = Pointer to buffer in RAM.
;Output:  A = 0: Ok, buffer filled with information.
;             1: Error, device or logical unit not available,
;                or device number or logical unit number invalid.
;         On success, buffer filled with the following information:
;
;+0 (1): Medium type:
;        0: Block device
;        1: CD or DVD reader or recorder
;        2-254: Unused. Additional codes may be defined in the future.
;        255: Other
;+1 (2): Sector size, 0 if this information does not apply or is
;        not available.
;+3 (4): Total number of available sectors.
;        0 if this information does not apply or is not available.
;+7 (1): Flags:
;        bit 0: 1 if the medium is removable.
;        bit 1: 1 if the medium is read only. A medium that can dinamically
;               be write protected or write enabled is not considered
;               to be read-only.
;        bit 2: 1 if the LUN is a floppy disk drive.
;+8 (2): Number of cylinders (0, if not a hard disk)
;+10 (1): Number of heads (0, if not a hard disk)
;+11 (1): Number of sectors per track (0, if not a hard disk)

NEXTOR2_LUN_INFO:
	call	CHECK_DEV_LUN
	jp	c,LUN_INFO_ERROR

	ld	b,a
	call	IDE_ON
	ld	a,b

	push	hl
	pop	ix

	dec	a
	jr	z,LUN_INFO2
	ld	a,M_DEV
LUN_INFO2:
	ld	e,a
	call	WAIT_CMD_RDY	
	jr	c,LUN_INFO_ERROR
	ld	a,e

	ld	(IDE_HEAD),a

	ld	a,0ECh
	call	DO_IDE
	jr	c,LUN_INFO_ERROR

	;Set cylinders, heads, and sectors/track

	ld	hl,(IDE_DATA)	;Skip word 0
	ld	hl,(IDE_DATA)
	ld	(ix+8),l	;Word 1: Cylinders
	ld	(ix+9),h
	ld	hl,(IDE_DATA)	;Skip word 2
	ld	hl,(IDE_DATA)
	ld	(ix+10),l	;Word 3: Heads
	ld	hl,(IDE_DATA)
	ld	hl,(IDE_DATA)	;Skip words 4,5
	ld	hl,(IDE_DATA)
	ld	(ix+11),l	;Word 6: Sectors/track

	;Set maximum sector number

	ld	b,60-7	;Skip until word 60
LUN_INFO_SKIP1:
	ld	de,(IDE_DATA)
	djnz	LUN_INFO_SKIP1

	ld	de,(IDE_DATA)	;DE = Low word
	ld	hl,(IDE_DATA)	;HL = High word

	ld	(ix+3),e
	ld	(ix+4),d
	ld	(ix+5),l
	ld	(ix+6),h

	;Set sector size

	ld	b,117-62	;Skip until word 117
LUN_INFO_SKIP2:
	ld	de,(IDE_DATA)
	djnz	LUN_INFO_SKIP2

	ld	de,(IDE_DATA)	;DE = Low word
	ld	hl,(IDE_DATA)	;HL = High word

	ld	a,h	;If high word not zero, set zero (info not available)
	or	l
	ld	hl,0
	jr	nz,LUN_INFO_SSIZE

	ld	a,d
	or	e
	jr	nz,LUN_INFO_SSIZE
	ld	de,512	;If low word is zero, assume 512 bytes
LUN_INFO_SSIZE:
	ld	(ix+1),e
	ld	(ix+2),d

	;Set other parameters

	ld	(ix),0	;Block device
	ld	(ix+7),0	;Non removable device nor LUN

	call	IDE_OFF
	xor	a
	ret

LUN_INFO_ERROR:
	call	IDE_OFF
	ld	a,1
	ret


;=======================
; Subroutines
;=======================

;-----------------------------------------------------------------------------
;
; Enable or disable the IDE registers

;Note that bank 7 (the driver code bank) must be kept switched

IDE_ON:
	ld	a,1+7*32
	ld	(IDE_BANK),a
	ret

IDE_OFF:
	ld	a,7*32
	ld	(IDE_BANK),a
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
	ld	de,8142		;Limit the wait to 30s
WAIT_RDY1:
	ld	b,255
WAIT_RDY2:
	ld	a,(IDE_STATUS)
	and	M_BSY+M_DRDY
	cp	M_DRDY
	jr	z,WAIT_RDY_END	;Wait for BSY to clear and DRDY to set		
	djnz	WAIT_RDY2	;End of WAIT_RDY2 loop
	dec	de
	ld	a,d
	or	e
	jr	nz,WAIT_RDY1	;End of WAIT_RDY1 loop
	scf
WAIT_RDY_END:
	pop	bc
	pop	de
	ret	
	
;-----------------------------------------------------------------------------
;
; Execute a command
;
; Input:  A = Command code
;         Other command registers appropriately set
; Output: Cy=1 if ERR bit in status register set

DO_IDE:
	ld	(IDE_CMD),a

WAIT_IDE:
	nop	; Wait 50us
	ld	a,(IDE_STATUS)
	bit	DRQ,a
	jr	nz,IDE_END
	bit	BSY,a
	jr	nz,WAIT_IDE

IDE_END:
	rrca
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
	push	af
	xor	a
	EX AF,AF'
	XOR A
	LD IX,GWORK
	call CALBNK
	pop	af
	cp	1
	ret	z
	inc	ix
	inc	ix
	inc	ix
	inc	ix
	ret


;-----------------------------------------------------------------------------
;
; Check the device number and LUN
; Input:  A = device number, B = lun
; Output: Cy=0 if OK, 1 if device or LUN invalid
;         IX = Work area for the device
; Modifies F, C

CHECK_DEV_LUN:
	or	a	;Check device number
	scf
	ret	z
	cp	3
	ccf
	ret	c

	ld	c,a
	ld	a,b	;Check LUN number
	cp	1
	ld	a,c
	scf
	ret	nz

	push	hl
	push	de
	call	MY_GWORK
	pop	de
	pop	hl
	ld	c,a
	ld	a,(ix)
	or	a
	ld	a,c
	scf
	ret	z

	or	a
	ret


;=======================
; Strings
;=======================

INFO_S:
	db	13,10,"Sunrise IDE driver v"
	db	VER_MAIN+"0",".",VER_SEC+"0",".",VER_REV+"0",13,10

ifdef MASTER_ONLY

	db "Master device only edition",13,10

endif

	db	"(c) Konamiman  2009",13,10
	db	"(c) Piter Punk 2014",13,10,13,10,0

SEARCH_S:
	db	"Searching: ",0

NODEVS_S:
	db	"Not found",13,10,0
MASTER_S:
	db	"Master device: ",0

ifndef MASTER_ONLY

SLAVE_S:
	db	"Slave device:  ",0

endif

CRLF_S:
	db	13,10,0


;-----------------------------------------------------------------------------
;
; Padding up to the required iver size

DRV_END:

	ds	3ED0h-(DRV_END-DRV_START)

	end
