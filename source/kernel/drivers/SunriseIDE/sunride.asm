	; Device-based driver for the sunrise IDE interface for Nextor
	;
    ; Version 0.1.7
    ; By Konamiman
	; By Piter Punk
	; By FRS

	org	4000h
	ds	4100h-$,0		; DRV_START must be at 4100h
DRV_START:

TESTADD	equ	0F3F5h

;-----------------------------------------------------------------------------
;
; Driver configuration constants
;

;Driver type:
;   0 for drive-based
;   1 for device-based

DRV_TYPE	equ	1

;Hot-plug devices support (device-based drivers only):
;   0 for no hot-plug support
;   1 for hot-plug support

DRV_HOTPLUG	equ	0

DEBUG	equ	0	;Set to 1 for debugging, 0 to normal operation

;Driver version

VER_MAIN	equ	0
VER_SEC		equ	1
VER_REV		equ	7


;Miscellaneous configuration
 DEFINE	PIOMODE3	; Configure devices to work on PIO MODE 3


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

M_ABRT	equ	100b ;(1 SHL ABRT)

; Bits in the head register

DEV	equ	4	;Device select: 0=master, 1=slave
LBA	equ	6	;0=use CHS mode, 1=use LBA mode

M_DEV	equ	10000b ;(1 SHL DEV)
M_LBA	equ	1000000b ;(1 SHL LBA)

; Bits in the status register

BSY	equ	7	;Busy
DRDY	equ	6	;Device ready
DF	equ	5	;Device fault
DRQ	equ	3	;Data request
ERR	equ	0	;Error

M_BSY	equ	10000000b ;(1 SHL BSY)
M_DRDY	equ	1000000b ;(1 SHL DRDY)
M_DF	equ	100000b ;(1 SHL DF)
M_DRQ	equ	1000b ;(1 SHL DRQ)
M_ERR	equ	1b ;(1 SHL ERR)

; Bits in the device control register register

nIEN	equ	1	;Disable interrupts
SRST	equ	2	;Software reset

M_nIEN	equ	10b ;(1 SHL nIEN)
M_SRST	equ	100b ;(1 SHL SRST)

; IDE commands

ATACMD:
.PRDSECTRT	equ	#20
.PWRSECTRT	equ	#30
.DEVDIAG	equ	#90
.IDENTIFY	equ	#EC
.SETFEATURES	equ	#EF

ATAPICMD:
.RESET		equ	#08
.PACKET		equ	#A0
.IDENTPACKET	equ	#A1

PACKETCMD:
.RQSENSE	equ	#03	;
.RDCAPACITY	equ	#25	; Read the media capacity
.READ10		equ	#28	; Read sectors (16bit)
.READ12		equ	#A8	; Read sectors (32bit)
.WRITE10	equ	#2A	; Write sectors (16bit)
.WRITE12	equ	#AA	; Write sectors (32bit)
.GTMEDIASTAT	equ	#DA	; Get media status


;-----------------------------------------------------------------------------
;
; Standard BIOS entries
CALSLT	equ	#001C
DCOMPR	equ	#0020		; Compare register pairs HL and DE
INITXT	equ	#006C
CHSNS	equ	#009C		; Sense keyboard buffer for character
CHGET	equ	#009F		; Get character from keyboard buffer
CHPUT	equ	#00A2		; A=char
BREAKX	equ	#00B7		; Check CTRL-STOP key directly
CLS	equ	#00C3		; Chamar com A=0
ERAFNK	equ	#00CC		; Erase function key display
SNSMAT	equ	#0141		; Read row of keyboard matrix
KILBUF	equ	#0156		; Clear keyboard buffer
EXTROM	equ	#015F
CHGCPU	equ	#0180
GETCPU	equ	#0183

; subROM functions
SDFSCR	equ	#0185
REDCLK	equ	#01F5

; System variables
MSXVER	equ	#002D
LINL40	equ	#F3AE		; Width
LINLEN	equ	#F3B0
BDRCLR	equ	#F3EB
BASROM	equ	#FBB1
INTFLG	equ	#FC9B
JIFFY	equ	#FC9E
SCRMOD	equ	#FCAF
EXPTBL	equ	#FCC1


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
;              00: Block device, non-removable
;              01: Block device, removable
;              10: Other, non removable
;              11: CD-ROM
;    bits 4,5: Device type for LUN 2 on master device
;    bits 6,7: Device type for LUN 3 on master device
;    Inside DRV_INIT only: b7=1 error detected on diagnostics
;                   b6~b0: reported error code
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


 STRUCT DEVINFO		; Contains information about a specific device
BASE			; Offset to the base of the data structure
t321D		db
t7654		db
;CHSRESERVED	ds 2	; Disabled to save space. 2 bytes won't be enough anyway
SECTSIZE	dw	; Sector size for this device
pBASEWRK	dw	; Cache pointer to go back to the base of the work area 
 ENDS

 STRUCT WRKAREA
BASE			; Offset to the base of the data structure 
BLKLEN		dw	; Size of the block to be copied. ***Must be
			; \the first element of the WRKAREA
PCTBUFF		ds 16	; Buffer to send ATAPI PACKET commands
LDIRHLPR	ds 8	; LDIR data transter helper routine. This is
			; in RAM to speed up the R800 copy a lot.
MASTER		DEVINFO	; Offset to the MASTER data structure
SLAVE		DEVINFO	; Offset to the SLAVE data structure
			; \*** It must follow the MASTER DEVINFO
 ENDS

 STRUCT WRKTEMP
pDEVMSG		dw	; Pointer to the text "Master:" or "Slave:"
BUFFER		ds 512	; Buffer for the IDENTIFY info
 ENDS

; ATAPI/SCSI packet structures
 STRUCT PCTRW10		; PACKET READ10/WRITE10 structure
OPCODE		db
OPTIONS		db
LBA		dd
GROUP		db
LENGHT		dw
CONTROL		db
 ENDS

 STRUCT PCTRW12		; PACKET READ12/WRITE12 structure
OPCODE		db
OPTIONS		db
LBA		dd
LENGHT		dd
GROUP		db
CONTROL		db
 ENDS




;-----------------------------------------------------------------------------
;
; Error codes for DEV_RW and DEV_FORMAT
;

_NCOMP	equ	0FFh
_WRERR	equ	0FEh
_DISK	equ	0FDh
_NRDY	equ	0FCh
_DATA	equ	0FAh
_RNF	equ	0F9h
_WPROT	equ	0F8h
_UFORM	equ	0F7h
_SEEK	equ	0F3h
_IFORM	equ	0F0h
_IDEVL	equ	0B5h
_IPARM	equ	08Bh

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


;* Set the current bank
; Must be used exclusively by the MSXBOOT routine
SETBNK	equ	7FD0h


;-----------------------------------------------------------------------------
;
; Built-in format choice strings
;

NULL_MSG  equ     741Fh	;Null string (disk can't be formatted)
SING_DBL  equ     7420h ;"1-Single side / 2-Double side"


;-----------------------------------------------------------------------------
;
; Driver signature
;
	db	"NEXTOR_DRIVER",0

; Driver flags:
;    bit 0: 0 for drive-based, 1 for device-based
;    bit 1: 1 for hot-plug devices supported (device-based drivers only)

	db 1+(2*DRV_HOTPLUG)

;Reserved byte
	db	0

;Driver name

DRV_NAME:
	db	"Sunrise IDE"
	ds	32-($-DRV_NAME)," "

;Jump table

	jp	DRV_TIMI
	jp	DRV_VERSION
	jp	DRV_INIT
	jp	DRV_BASSTAT
	jp	DRV_BASDEV
        jp      DRV_EXTBIO
        jp      DRV_DIRECT0
        jp      DRV_DIRECT1
        jp      DRV_DIRECT2
        jp      DRV_DIRECT3
        jp      DRV_DIRECT4

	ds	15

	jp	DEV_RW
	jp	DEV_INFO
	jp	DEV_STATUS
	jp	LUN_INFO
	jp	DEV_FORMAT
	jp	DEV_CMD


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

DRV_INIT:
	ld	hl,WRKAREA	; size of work area
	or	a		; Clear Cy
	ret	z

; 2nd call: 
	ld	a,(CHGCPU)
	cp	#C3		; IS CHGCPU present?
	jr	nz,.call2ini
	call	GETCPU
	push	af		; Save the current CPU
	ld	a,#82
	call	CHGCPU		; Enable the turbo
.call2ini:
	call	MYSETSCR

	ld	de,INFO_S
	call	PRINT

	xor	a			; Request the WorkArea base pointer
	call	MY_GWORK
	call	INIWORK			; Initialize the work-area
	call	IDE_ON

.init:	ld	(ix+WRKAREA.MASTER.t321D),#FE	; error: No master detected yet
	ld	de,INIT_S		; Print "Initializing: "
	call	PRINT
	ld	a,M_DEV			; Select SLAVE
	call	WAIT_BSY		; Is the it alive?
	jr	c,.reset		; No, reset everyone
	xor	a			; select MASTER
	call	SELDEV
	call	WAIT_BSY		; Is the it alive?
	jr	nc,.diag		; Yes, skip
.reset:
	call	RESET_ALL
.diag:
	ld	a,(INTFLG)
	cp	3			; CTRL+STOP pressed?
	jp	z,INIT_ABORTED
	ld	a,ATACMD.DEVDIAG	; Both drives will execute diagnostics
	ld	(IDE_CMD),a
	call	WAIT_RST		; Wait for the diagnostics to end
	ld	a,(INTFLG)
	cp	3			; CTRL+STOP pressed?
	jp	z,INIT_ABORTED
	ld	a,(IDE_STATUS)
	and	M_ERR			; Error bit set?
	jr	nz,.diagchk		; on error, skip to diagnostics
	ld	(ix+WRKAREA.MASTER.t321D),0	; Clear undetected master error

.diagchk:
	; Check the diagnostics and print the results 
	call	CHKDIAG
	push	af
	ld	de,INIT_S		; Print "Initializing: "
	call	PRINT
	pop	af
	ld	de,OK_S
	call	nc,PRINT
	ld	de,ERROR_S
	call	c,PRINT

.chkmaster:
	ld	de,MASTER_S
	ld	(TEMP_WORK.pDEVMSG),de
	call	PRINT

	; Check for DIAGNOSTICS errors
	ld	a,(ix+WRKAREA.MASTER.t321D)
	ld	c,a
	bit	7,a			; Any error detected by DIAGNOSE?
	jr	z,.detinit		; No, skip
	call	DIAGERRPRT		; Print the diagnostic error
	ld	(ix+WRKAREA.MASTER.t321D),0	; This device isn't available
	; Errors 0 and 5 are critical and cannot proceed
	ld	a,c
	and	#7F			; Crop erro code
	jr	z,.critical
	cp	5			; Microcontroller error on master?
	jp	nz,.chkslave		; No: slave can still be used safely
.critical:
	ld	(ix+WRKAREA.SLAVE.t321D),0	; No master = no slave
	jp	DRV_INIT_END		; Finish DEV_INIT

.detinit:
	ld	a,M_DEV			; Select SLAVE
	call	SELDEV
	call	WAIT_RST		; wait until ready
	jr	nc,.detmaster
	ld	(ix+WRKAREA.SLAVE.t321D),#80	; Slave has an error

.detmaster:
	call	RESET_ALL.ataonly
	push	ix
	ld	de,WRKAREA.MASTER
	add	ix,de			; Point ix to the MASTER work area
	xor	a			; Select MASTER
	call	DETDEV
	pop	ix
	ld	a,(ix+WRKAREA.MASTER.t321D)
	and	3		; There can't be a slave without a master
	jr	z,INIT_MASTERFAIL	; Finish DEV_INIT

.chkslave:
	ld	de,SLAVE_S
	ld	(TEMP_WORK.pDEVMSG),de
	call	PRINT

	; Check for DIAGNOSTICS errors
	ld	a,(ix+WRKAREA.SLAVE.t321D)
	bit	7,a			; Any error detected by DIAGNOSE?
	jr	z,.detslave		; No, skip to detection

	call	DIAGERRPRT		; Print the diagnostic error
	ld	(ix+WRKAREA.SLAVE.t321D),0	; This device isn't available
	jp	DRV_INIT_END

.detslave:
	push	ix
	ld	de,WRKAREA.SLAVE.BASE
	add	ix,de			; Point ix to the SLAVE work area
	ld	a,M_DEV			; Select SLAVE
	call	DETDEV
	pop	ix

	; Reset all devices to finish
END_DETECT:
	call	RESET_ALL

	;--- End of the initialization procedure
DRV_INIT_END:
	call	IDE_OFF
	call	INICHKSTOP
	ld	de,CRLF_S	; Skip a line for the next driver
	call	PRINT

	; ***Workaround for a bug in Nextor that causes it to freeze if
	; CTRL+STOP was pressed on boot
	ld	a,(INTFLG)
	cp	3		; Is CTRL+STOP still signaled?
	jr	nz,.restCPU	; no, skip
	xor	a
	ld	(INTFLG),a	; Clear CTRL+STOP otherwise Nextor will freeze

.restCPU:	; Restore the CPU if necessary
	ld	a,(CHGCPU)
	cp	#C3		; IS CHGCPU present?
	ret	nz
	pop	af
	or	#80
	jp	CHGCPU


INIT_ABORTED:
	ld	de,ABORTED_S		; Print "<aborted>"
	jr	INIT_MASTERFAIL.end

INIT_MASTERFAIL:
	ld	de,DIAGS_S.nomaster	; Print "failed">
	call	PRINT
.end:	ld	(ix+WRKAREA.MASTER.t321D),0
	ld	(ix+WRKAREA.SLAVE.t321D),0
	jr	DRV_INIT_END




;--- Subroutines for the INIT procedure

; Input: A=target device
;        (TEMP_WORK.pDEVMSG): Pointer to the "Master" or "Slave" text
DETDEV:
	ld	c,a			; c=target device
	ld	de,DETECT_S		; Print "detecting"
	call	PRINT

	ld	a,(INTFLG)
	cp	3			; Was CTRL+STOP signaled?
        jp	z,.aborted		; Yes, skip

	ld	a,c			; Select device
	call	SELDEV
	call	WAIT_BSY		; wait until ready
	jp	c,.nodev
 	call	DISDEVINT		; Disable interrupts
	jp	c,.nodev

	ld      a,'.'			; Print the FIRST dot
	call    CHPUT
	;--- Get the device type 
	call	GETDEVTYPE		; Get the device type
	ei
	ld	(ix+DEVINFO.t321D),a
	jp	c,.nodev
	cp	#FF
	jp	z,.unknown

	;---Configure the PIO transfer mode
 IFDEF PIOMODE3
	ld	a,3			; Set transfer mode
	ld	(IDE_FEAT),a
	
	ld	a,#8+#3			; PIO flow control, mode 3, so
					; IORDY becomes functional
	ld	(IDE_SECCNT),a

	ld	a,ATACMD.SETFEATURES
	ld	(IDE_CMD),a
	call	WAIT_BSY
	jr	c,.unsupported		; No PIO3? This device is too old
 ENDIF

	;--- Get the name of the device 
	;(IDENTIFY device data on TEMP_WORK.BUFFER)
.getinfo:
	ld	de,(TEMP_WORK.pDEVMSG)
	call	PRINT

	;Print the device name.
	ld	hl,TEMP_WORK.BUFFER+27*2
	ld	b,20
	call	.prtword

	; Print the firmware version
	ld	hl,TEMP_WORK.BUFFER+23*2
	ld	b,4
	call	.prtword

	; Print the device characteristics
	ld	de,DETECT_S.oparenthesis
	call	PRINT
	ld	a,(ix+DEVINFO.t321D)
	and	3			; Crop devtype
	ld	de,DETECT_S.chs
	dec	a
	jr	z,.printdevtype
	ld	de,DETECT_S.lba
	dec	a
	jr	z,.printdevtype
	ld	de,DETECT_S.atapi
.printdevtype:
	call	PRINT

	ld	a,')'
	call	CHPUT
	ld	de,CRLF_S
	jp	PRINT





	; --- Print an word string
	; input: HL=string ponter
	;         B=Number of words to print 
.prtword:
	ld	c,(hl)
	inc	hl
	ld	a,(hl)
	inc	hl
	call	CHPUT
	ld	a,c
	call	CHPUT
	djnz	.prtword
	ret

	;--- Unknown device
.unknown:
	ld	(ix+DEVINFO.t321D),a
	ld	de,DETECT_S.unknown
	call	PRINT
	ld	a,h
	call	PRINTHEXBYTE
	ld	a,l
	call	PRINTHEXBYTE
	ld	a,'h'
	call	CHPUT
	ld	de,CRLF_S
	jp	PRINT

	;--- Unsupported device (either too old or too new)
.unsupported:
	ld	de,DETECT_S.unsupported
	jr	.nodevreason

	;--- No device was found
.nodev:
	ld	de,NODEVS_S

	;--- Prints the reason why there will be no device reported here
.nodevreason:
	ld	(ix+DEVINFO.t321D),0	
	push	de
	ld	de,(TEMP_WORK.pDEVMSG)
	call	PRINT			; Clear previous label message
	pop	de
	jp	PRINT

	;--- Dectection aborted by the user
.aborted:
	ld	de,ABORTED_S
	jr	.nodevreason

;-----------------------------------------------------------
WAIT_RST:
; Wait for the BSY flat to be reset for the selected device
; Output: Cy set= timeout
;         (INTFLG)=3 if the user pressed CTRL+STOP
; Note: This routine is intended to wait BSY for long periods, as required
; by soft-reset and DIAGNOSTICS. 

; Step-1: Fast reset
; Will catch devices that reset quicly, like CFs and HDDs that are already spinning

	ld	de,5*60			; Wait for up to 5s
.wait1:
	ld	a,e
	and	127			; Is it the time to print a dot?
        ld      a,'.'
        call    z,CHPUT			;Yes, print dots while waiting
	call	BREAKX
	ret	c
        ld      bc,#0505		;5 fast retries for 5 short pauses 
.wait2:
	ld	a,(INTFLG)
	cp	3			; was CTRL+STOP still signaled?
	ret	z
        ld      a,(IDE_STATUS)
	and	M_BSY
	ret	z		 	; This ATA device has finished its reset
        djnz    .wait2
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	dec	c
        jr	nz,.wait2
	dec	de
	ld	a,e
	or	d
	halt				;Give the device some time to breath
	jr	nz,.wait1
	scf
	ret

;Check that a device is present and usable.
;Input:  DIAGNOSTICS issued successfully.
;Output: A=devtype. 0=nodev, 1=CHS, 2=LBA, 3=ATAPI
;	 Cy=0 for device ok, 1 for no device or not usable.
;        If device ok, 50 first bytes of IDENTIFY device copied to TEMP_WORK.

GETDEVTYPE:
; Input: none
; Output: A=devtype. 0=none, 1=CHS, 2=LBA, 3=ATAPI
;         Cy on if the device is undetected or unknown
;         When Cy=on, HL will contain the signature of the unknown device

; Notes:
	; Device Signatures are checked according to the ATA/ATAPI Command Set-3
	; (ACS-3) revision-5 document, page-347.

	;	 	PATA	PATAPI	SATA	SATAPI
	; COUNT    :	01h	01h	01h	01h
	; LBA 23-16:	00h	EBh	C3h	96h
	; LBA  8-15:	00h	14h	3Ch	69h
	; LBA   0-7:	01h	01h	01h	01h

	; This signatures are output by the following commands:
	; - DEVICE RESET	(08h)
	; - DEVICE DIAGNOSTIC	(90h)
	; - IDENTIFY DEVICE	(ECh)
	; - READ SECTOR(S)	(20h)

	; Note by Piter: Not all devices respect this. One of my CompactFlash
	; cards never have  Sector Count = 01 after reset.
	;ld	a,(IDE_SECCNT)
	;cp	1
	;jr	nz,INIT_CHECK_NODEV

	ld	hl,(IDE_LBAMID)
	ld	de,#EB14		; PATAPI signature?
	rst	DCOMPR
	ld	a,3			; devtype=ATAPI
	jr	z,.identify

	ld	de,#9669		; SATAPI signature?
	rst	DCOMPR
	ld	a,3			; devtype=ATAPI
	jr	z,.identify

	ld	de,#0000		; PATA signature?
	rst	DCOMPR
	ld	a,1			; devtype=CHS
	jr	z,.identify

	ld	de,#C33C		; SATA signature?
	rst	DCOMPR
	ld	a,2			; devtype=LBA
	jr	z,.identify

	ld	de,#7F7F		; Unconnected signature?
	rst	DCOMPR
	scf
	ret	z			; Yes, quit with Cy

	ld	de,#FFFF		; Unconnected signature?
	rst	DCOMPR
	scf
	ret	z			; Yes, quit with Cy

.unkdev:	; Unknown device
	ld	a,#FF
	or	a
	ret

.identify:
	ld	b,a			; b=devtype
	cp	3			; ATAPI?
	ld	a,ATACMD.IDENTIFY	;Send IDENTIFY commad
	jr	c,.identify2
	ld	a,ATAPICMD.IDENTPACKET	;Send IDENTIFY PACKET commad
.identify2:
	di
	call	PIO_CMD			
	ld	a,0			; Return 0 on error
	ret	c
	ld	a,b			; a=devtype
	ld	hl,IDE_DATA
	ld	de,TEMP_WORK.BUFFER
	ld	bc,512			;Read the IDENTIFY packet
	ldir
	ei
	cp	2			; SATA or ATAPI?
	ret	nc			; Yes, return

.chkLBA:
	ld      a,(TEMP_WORK.BUFFER+49*2+1)
	and	2			; LBA supported?
	ld	a,1			; devtype=CHS
	ret	z			; No, return
	inc	a			; devtype=LBA
	ret




;-----------------------------------------------------------------------------
;
; Obtain driver version
;
; Input:  -
; Output: A = Main version number
;         B = Secondary version number
;         C = Revision number

DRV_VERSION:
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


;=====
;=====  BEGIN of DEVICE-BASED specific routines
;=====

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
;              _IDEVL: Invalid device or LUN
;              _NRDY: Not ready
;              _DISK: General unknown disk error
;              _DATA: CRC error when reading
;              _RNF: Sector not found
;              _UFORM: Unformatted disk
;              _WPROT: Write protected media, or read-only logical unit
;              _WRERR: Write error
;              _NCOMP: Incompatible disk
;              _SEEK: Seek error
;          B = Number of sectors actually read/written

DEV_RW:
	push	af
	call	MY_GWORK	; ix=Work area pointer for this device

	push	bc
	ld	b,c		;b=LUN
	call	CHECK_DEV_LUN
	pop	bc
	jp	c,DEV_RW_NODEV

	dec	a
	jr	z,DEV_RW2
	ld	a,M_DEV
DEV_RW2:
	ld	c,a		;c=dev# in IDE format

	ld	a,b
	or	a
	jr	nz,DEV_RW_NO0SEC
	pop	af		;Discard the device number that was on stack
	xor	a
	ld	b,0
	ret	
DEV_RW_NO0SEC:
	ld	iy,de
	ld	a,(iy+3)
	and	11110000b
	jp	nz,DEV_RW_NOSEC	;Only 28 bit sector numbers supported

	call	IDE_ON

	ld	a,(ix+DEVINFO.t321D)
	and	3			; Crop devtype
	cp	3			; ATAPI?
	jp	z,DEV_ATAPI_RW

DEV_ATA_RW:
	ld	a,(iy+3)
	or	M_LBA
	or	c		; Mix with dev# in IDE format
	call	SELDEV		; IDE_HEAD must be written first,
	ld	a,(iy)		; or the other IDE_LBAxxx and IDE_SECCNT
	ld	(IDE_LBALOW),a	; registers will not get a correct value
	ld	e,(iy+1)	; (blueMSX issue?)
	ld	d,(iy+2)
	ld	(IDE_LBAMID),de
	ld	a,b
	ld	(IDE_SECCNT),a
	
	pop	af		; a=device number in Nextor format
	jp	c,DEV_ATA_WR

	;---
	;---  ATA READ
	;---
DEV_ATA_RD:
	ld	a,ATACMD.PRDSECTRT	; PIO read sector with retry
	call	PIO_CMD
	jp	c,DEV_RW_ERR

	call	CHK_RW_FAULT
	ret	c
	ld	iyl,b		; iyl=number of blocks
	ex	de,hl		; de=destination address

	ld	bc,512		; block size  ***Hardcoded. Ignores (BLKLEN)
	call	READ_DATA
	jp	c,DEV_RW_ERR
	call	IDE_OFF
	xor	a		; A=0: No error.
	ret

	;---
	;---  ATA WRITE
	;---
DEV_ATA_WR:
	ld	a,ATACMD.PWRSECTRT	; PIO write sector with retry
	call	PIO_CMD
	jp	c,DEV_RW_ERR
	ld	iyl,b		; iyl=number of blocks

	ld	bc,512		; block size  ***Hardcoded. Ignores (BLKLEN)
	call	WRITE_DATA
	jp	c,DEV_RW_ERR
	call	IDE_OFF
	xor	a		; A=0: No error.
	ret


DEV_ATAPI_RW:
	ld	a,c			;Get devnum in IDE format
	call	SELDEV

	; Fill the READ10/WRITE10 packet structure
	push	de
	ld	e,(ix+DEVINFO.pBASEWRK)		; hl=pointer to WorkArea
	ld	d,(ix+DEVINFO.pBASEWRK+1)
	ld	iy,de				; iy=WRKAREA pointer
	pop	de

	; Set the block size
	ld	a,(ix+DEVINFO.SECTSIZE)
	ld	(iy+WRKAREA.BLKLEN),a	
	ld	a,(ix+DEVINFO.SECTSIZE+1)
	ld	(iy+WRKAREA.BLKLEN+1),a

	ld	(iy+WRKAREA.PCTBUFF+PCTRW10.LENGHT),0
	ld	(iy+WRKAREA.PCTBUFF+PCTRW10.LENGHT+1),b
	ld	a,(de)
	ld	(iy+WRKAREA.PCTBUFF+PCTRW10.LBA+3),a
	inc	de
	ld	a,(de)
	ld	(iy+WRKAREA.PCTBUFF+PCTRW10.LBA+2),a
	inc	de
	ld	a,(de)
	ld	(iy+WRKAREA.PCTBUFF+PCTRW10.LBA+1),a
	inc	de
	ld	a,(de)
	ld	(iy+WRKAREA.PCTBUFF+PCTRW10.LBA+0),a
	; Set the fields that we don't need to 0
	xor	a
	ld	(iy+WRKAREA.PCTBUFF+PCTRW10.OPTIONS),a
	ld	(iy+WRKAREA.PCTBUFF+PCTRW10.GROUP),a
	ld	(iy+WRKAREA.PCTBUFF+PCTRW10.CONTROL),a
	ld	(iy+WRKAREA.PCTBUFF+PCTRW12.GROUP),a
	ld	(iy+WRKAREA.PCTBUFF+PCTRW12.CONTROL),a

	ld	de,512			;Set the buffer size to the 512 bytes
	ld	(IDE_LBAMID),de

	;
	pop	af		; a=device number in Nextor format, f=r/w flag
	jp	c,DEV_ATAPI_WR
	;

DEV_ATAPI_RD:
	ld	a,PACKETCMD.READ12
	ld	(iy+WRKAREA.PCTBUFF+PCTRW10.OPCODE),a
	ld	a,ATAPICMD.PACKET	; PIO send PACKET command 
	call	PIO_CMD
	jp	c,DEV_RW_ERR
	push	bc,hl,iy
	ld	iyl,1			; 1 block
	ld	hl,WRKAREA.PCTBUFF
	ld	bc,PCTRW10		; block size=10 bytes
	call	WRITE_DATA		; Send the packet to the device
	pop	iy,hl,bc
	jp	c,DEV_RW_ERR

.init1:	; Set the sector size and number of blocks
	; sizes bigger than 512 must be a multiple of 512
	ld	a,(ix+DEVINFO.SECTSIZE+1)
	srl	a		; SECTSIZE=SECTSIZE/512 (the IDE buffer can
				; transfer only 512 bytes per time)
	ld	de,512		; block size=512
	jr	nz,.init2	; Skip if the boundary check is ok
	inc	a		; Keep the block count to at least 1
	ld	e,(ix+DEVINFO.SECTSIZE)
	ld	d,(ix+DEVINFO.SECTSIZE+1)

.init2:	
	ld	c,a		; c=number of 512-byte blocks per sector

	push	bc
	ld	bc,de		; bc=block size
	call	SETLDIRHLPR	; hl'=Pointer to LDIR helper in RAM
	pop	bc
	ex	de,hl		; de=destination address
.loopsector:
	push	bc
	ld	iyl,c		; get the number of blocks per sector
.loopblock:
	call	WAIT_DRQ
	jr	c,.rderr
	ld	hl,IDE_DATA
	call	RUN_HLPR
	dec	iyl
	jr	nz,.loopblock
	pop	bc
	djnz	.loopsector

	call	IDE_OFF
	xor	a
	ret

.rderr:	; Allows the read loop to run faster with a jr
	jp	DEV_RW_ERR


DEV_ATAPI_WR:
	ld	a,PACKETCMD.WRITE10
	ld	(iy+WRKAREA.PCTBUFF+PCTRW10.OPCODE),a
	ld	a,ATAPICMD.PACKET	; PIO send PACKET command 
	call	PIO_CMD
	jp	c,DEV_RW_ERR
	push	bc,hl,iy
	ld	iyl,1			; 1 block
	ld	hl,WRKAREA.PCTBUFF
	ld	bc,PCTRW10		; block size=10 bytes
	call	WRITE_DATA		; Send the packet to the device
	pop	iy,hl,bc
	jp	c,DEV_RW_ERR

.init1:	; Set the sector size and number of blocks
	; sizes bigger than 512 must be a multiple of 512
	ld	a,(ix+DEVINFO.SECTSIZE+1)
	srl	a		; SECTSIZE=SECTSIZE/512 (the IDE buffer can
				; transfer only 512 bytes per time)
	ld	de,512		; block size=512
	jr	nz,.init2	; Skip if the boundary check is ok
	inc	a		; Keep the block count to at least 1
	ld	e,(ix+DEVINFO.SECTSIZE)
	ld	d,(ix+DEVINFO.SECTSIZE+1)

.init2:	
	ld	c,a		; c=number of 512-byte blocks per sector

	push	bc
	ld	bc,de		; bc=block size
	call	SETLDIRHLPR	; hl'=Pointer to LDIR helper in RAM
	pop	bc
.loopsector:
	push	bc
	ld	iyl,c		; get the number of blocks per sector
.loopblock:
	call	WAIT_DRQ
	jr	c,.rderr
	ld	de,IDE_DATA
	call	RUN_HLPR
	call	CHK_RW_FAULT
	jr	c,.rderr
	dec	iyl
	jp	nz,.loopblock
	pop	bc
	djnz	.loopsector

	call	IDE_OFF
	xor	a
	ret

.rderr:	; Allows the read loop to run faster with a jr
;	jp	DEV_RW_ERR


	;---
	;---  ERROR ON READ/WRITE
	;---

DEV_RW_ERR:
	ld	a,(IDE_ERROR)
	ld	b,a		; b=IDE_ERROR
	call	IDE_OFF
	bit	NM,b		;Not ready
	jr	nz,DEV_R_ERR1
	ld	a,_NRDY
	ld	b,0
	ret
DEV_R_ERR1:
	bit	IDNF,b		;Sector not found
	jr	nz,DEV_R_ERR2
	ld	a,_RNF
	ld	b,0
	ret
DEV_R_ERR2:
	bit	WP,b		;Write protected
	jr	nz,DEV_R_ERR3
	ld	a,_WPROT
	ld	b,0
	ret
DEV_R_ERR3:
	ld	a,_DISK		;Other error
	ld	b,0
	ret

	;--- Termination points

DEV_RW_NOSEC:
	call	IDE_OFF
	pop	af
	ld	a,_RNF
	ld	b,0
	ret

DEV_RW_NODEV:
	call	IDE_OFF
	pop	af
	ld	a,_IDEVL
	ld	b,0
	ret


;-----------------------------------------------------------------------------
;
; Device information gathering
;
;Input:   A = Device index, 1 to 7
;         B = Information to return:
;             0: Basic information
;             1: Manufacturer name string
;             2: Device name string
;             3: Serial number string
;         HL = Pointer to a buffer in RAM
;Output:  A = Error code:
;             0: Ok
;             1: Device not available or invalid device index
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

DEV_INFO:
	;Check device index boundaries
	or	a
	jp	z,.error1
	cp	3
	jp	nc,.error1

	call	MY_GWORK

	ld	c,a
	ld	a,b
	or	a
	jr	nz,.strings

	;--- Obtain basic information

	ld	a,(ix+DEVINFO.t321D)	; Get current device type
	and	3			;Device available? 
	jr	z,.error1

	ld	(hl),1			;One single LUN
	inc	hl
	ld	(hl),0			;Always zero
	xor	a
	ret

	;--- Obtain string information
.strings:
	call	IDE_ON

	ld	a,c
	dec	a
	jr	z,.swcase
	ld	a,M_DEV

.swcase:
	call	SELDEV

	ld	a,b
	dec	a			; A=1? (Manufacturer name)
	jr	z,.error2		; Yes, quit. IDE doesn't have it.

	dec	a			; A=2? (Device name)
	jr	z,.devname

	dec	a			; A=3? (Serial number)
	jr	nz,.error2		; No, quit with error
	jr	.devserial		; Skip to serial number routine

	;--- Device name
.devname:
;	push	hl
	call	DEV_STRING_CLR
	ld	bc,#1B14		; Device name word on IDENTIFY
	call	DEV_STRING_DIGEST
;	pop	hl
	jr	c,.error1
;	ld	bc,#1708
;	ld	de,21
;	add	hl,de
;	call	DEV_STRING_DIGEST	; Get the device version
;	jr	c,.error1

	call	IDE_OFF
	xor	a
	ret


	;--- Serial number
.devserial:
	call	DEV_STRING_CLR
	ld	bc,#0A0A
	ld	de,44
	add	hl,de			;Since the string is 20 chars long
	call	DEV_STRING_DIGEST
	jr	c,.error1
	call	IDE_OFF
	xor	a
	ret

	
	;--- Termination with error
.error1:
	call	IDE_OFF
	ld	a,1
	scf
	ret

.error2:
	call	IDE_OFF
	ld	a,2
	ret


;--- Clear the destination buffer
; Input   : HL = 64 bytes string buffer
; Modifies: AF, BC 
DEV_STRING_CLR:
	push	hl
	ld	a,' '
	ld	b,64
.loopclr:
	ld	(hl),a
	inc	hl
	djnz	.loopclr
	pop	hl
	ret





;Common processing for obtaining a device information string
;Input   :  B = Offset of the string in the device information (words)
;           C = Size of the string to be copied to the buffer (bytes)
;          HL = Destination address for the string
;Modifies: AF, BC, DE, HL
DEV_STRING_GET:
	push	hl
	; Calculate the number of bytes that will remain
	ld	a,c
	srl	a			; a=number of words in the string
	add	b			; a=number of words to be consumed 
	ld	e,a
	ld	d,0
	ld	hl,256
	or	a
	sbc	hl,de
	ld	h,l			; h=number of remaining words
	ex	(sp),hl			; (sp)=number of remaining words

	ld	a,(ix+DEVINFO.t321D)
	and	3
	cp	3			; ATAPI?
	ld	a,ATACMD.IDENTIFY	; Send IDENTIFY commad
	jr	c,.identify
	ld	a,ATAPICMD.IDENTPACKET	;Send IDENTIFY PACKET commad
.identify:
	call	PIO_CMD
	jr	c,.errorpop

	ex	de,hl			; de=string buffer
.skip:
	ld	hl,(IDE_DATA)	;Skip device data until the desired string
	djnz	.skip

	; Transfer all bytes to the buffer. String processing must be done
	; later because some devices don't like when the transfer is too slow
;	ld	b,0
	ld	hl,IDE_DATA
	ldir

	pop	bc			; b=number of remaining words
.flushloop:	; Flush the rest of the data
	ld	hl,(IDE_DATA)
	djnz	.flushloop

	or	a			; Clear Cy
	ret

.errorpop:
	pop	de			; Discard stack data
	ret


; Digest a string from the device and place it on the buffer
; Input   : IY=Pointer to the text buffer
;   	     B=Size of the string
; Modifies: AF, BC
DEV_STRING_DIGEST:
	push	bc,hl
	call	DEV_STRING_GET
	pop	iy,bc
	ret	c
	ld	b,c

	; --- Digest an ATA string into a text string
.stringloop:
	ld	a,(iy+1)
	ld	c,(iy)
	call	.validatechar	
	ld	(iy),a
	ld	a,c
	call	.validatechar	
	ld	(iy+1),a
	inc	iy
	inc	iy
	djnz	.stringloop
	or	a			; Clear Cy
	ret

.validatechar:
	cp	32
	jr	c,.invalidchar
	cp	127
	ret	c
.invalidchar:
	ld	a,'_'
	ret




;-----------------------------------------------------------------------------
;
; Obtain device status
;
;Input:   A = Device index, 1 to 7
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
;                    assigned the same device index; for logical units,
;                    the media has been changed).
;                3: The device or logical unit is available, but it is not
;                   possible to determine whether it has been changed
;                   or not since the last status request.
;
; Devices not supporting hot-plugging must always return status value 1.
; Non removable logical units may return values 0 and 1.

DEV_STATUS:
	set	0,b	;So that CHECK_DEV_LUN admits B=0

	call	CHECK_DEV_LUN
	ld	e,a
	ld	a,0
	ret	c

	ld	a,1	;Never changed
	ret

	;ld	a,1
	;ret

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
;Input:   A  = Device index, 1 to 7.
;         B  = Logical unit number, 1 to 7.
;         HL = Pointer to buffer in RAM.
;Output:  A = 0: Ok, buffer filled with information.
;             1: Error, device or logical unit not available,
;                or device index or logical unit number invalid.
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

LUN_INFO:
	call	CHECK_DEV_LUN
	jp	c,LUN_INFO_ERROR

	call	MY_GWORK		; ix=workarea for this device

	ld	b,a
	call	IDE_ON
	ld	a,b

	push	hl
	pop	iy
	dec	a
	jr	z,LUN_INFO2
	ld	a,M_DEV
LUN_INFO2:

	ld	e,a
	call	WAIT_DRDY
	jp	c,LUN_INFO_ERROR
	ld	a,e

	call	SELDEV

	ld	a,(ix+DEVINFO.t321D)
	and	3
	cp	3			; ATAPI?
	jp	z,LUN_NFO_ATAPI		; Yes, skip

	ld	a,ATACMD.IDENTIFY	; Send IDENTIFY commad
	call	PIO_CMD
	jp	c,LUN_INFO_ERROR

	;========== Device properties ==========

	;---Set the device type
	ld	(iy),0		;set device type
	ld	(iy+7),0	;Not removable, nor floppy

	ld	hl,(IDE_DATA)	;Skip word 0
	;Set cylinders, heads, and sectors/track
	ld	hl,(IDE_DATA)
	ld	(iy+8),l	;Word 1: Cylinders
	ld	(iy+9),h
	ld	hl,(IDE_DATA)	;Skip word 2
	ld	hl,(IDE_DATA)
	ld	(iy+10),l	;Word 3: Heads
	ld	hl,(IDE_DATA)
	ld	hl,(IDE_DATA)	;Skip words 4,5
	ld	hl,(IDE_DATA)
	ld	(iy+11),l	;Word 6: Sectors/track

	;Set maximum sector number
	ld	b,60-7	;Skip until word 60
.skip1:
	ld	de,(IDE_DATA)
	djnz	.skip1

	ld	de,(IDE_DATA)	;DE = Low word
	ld	hl,(IDE_DATA)	;HL = High word

	ld	(iy+3),e
	ld	(iy+4),d
	ld	(iy+5),l
	ld	(iy+6),h

	;Set sector size
	ld	b,117-62	;Skip until word 117
.skip2:
	ld	de,(IDE_DATA)
	djnz	.skip2

	ld	de,(IDE_DATA)	;DE = Low word
	ld	hl,(IDE_DATA)	;HL = High word

	ld	a,h	;If high word not zero, set zero (info not available)
	or	l
	ld	hl,0
	ex	de,hl
	jr	nz,.info_ssize

	ex	de,hl
	ld	a,d
	or	e
	jr	nz,.info_ssize
	ld	de,512	;If low word is zero, assume 512 bytes
.info_ssize:
	ld	(iy+1),e
	ld	(iy+2),d
	ld	(ix+DEVINFO.SECTSIZE),e
	ld	(ix+DEVINFO.SECTSIZE+1),d

	;Flush the rest of the data
	ld	b,256-118
.skip3:
	ld	de,(IDE_DATA)
	djnz	.skip3

	; Finish
	call	IDE_OFF
	xor	a
	ret

LUN_NFO_ATAPI:
	ld	a,ATAPICMD.IDENTPACKET	;Send IDENTIFY PACKET commad
	call	PIO_CMD
	jp	c,LUN_INFO_ERROR

	ld	hl,(IDE_DATA)	;Read word 0
	; Get the ATAPI device type
	xor	a
	sla	l		;Get the removable-device flag
	adc	a,a		;Inject it in A
	ld	(iy+7),a
	ld	a,h
	and	#1F		;Crop command packet set
	ld	d,a		;d=0: block device
	jr	z,.setdevtype	;Direct-access device
	dec	d		;d=255: other
	cp	5		;CD-ROM?
	jr	nz,.setdevtype	;Yes, skip
	ld	d,1		;d=1: CD-ROM
	ld	a,2		;read-only media
	or	(iy+7)
.setdevtype:
	ld	(iy+7),a
	ld	(iy),d		;set device type

	ld	b,255		;Flush the rest of the data
.skip1:
	ld	hl,(IDE_DATA)
	djnz	.skip1


	ld	hl,512			;Set the buffer size to 512 bytes
	ld	(IDE_LBAMID),hl	
	ld	l,(ix+DEVINFO.pBASEWRK)		; hl=WorkArea
	ld	h,(ix+DEVINFO.pBASEWRK+1)
	ld	de,WRKAREA.PCTBUFF
	add	hl,de
	push	hl
	ld	(hl),PACKETCMD.RDCAPACITY
	inc	hl
	ld	b,11
.zloop:	ld	(hl),0		; Clear the rest of the package
	inc	hl
	djnz	.zloop

	ld	a,ATAPICMD.PACKET	; PIO send PACKET command 
	call	PIO_CMD
	jr	c,.errorpop

	pop	hl
	push	hl		; Source=PCTBUF
	ld	bc,12		; 12 byte packet
	push	iy
	ld	iyl,1
	call	WRITE_DATA	; Send the packet to the device
	pop	iy
	jr	nc,.rdmediapropr	; No error? Then read media proprieties

	; Guess the sector size based on the device type
	pop	de		; Discard the destination buffer address
	ld	a,(iy)		; Get device type
	ld	de,2048		; CD-ROM sector size
	cp	1		; CD-ROM?
	jr	z,.info_ssizeatapi
	ld	de,512		; Otherwise assume a 512 byte sector
	jr	.info_ssizeatapi

.rdmediapropr:
	pop	de		; Destination=PCTBUFF
	ld	bc,8		; 8 byte response
	push	iy
	ld	iyl,1
	call	READ_DATA
	pop	iy
	jr	c,LUN_INFO_ERROR

	ld	h,(ix+WRKAREA.PCTBUFF+0)	; Get the number of sectors
	ld	l,(ix+WRKAREA.PCTBUFF+1)
	ld	d,(ix+WRKAREA.PCTBUFF+2)
	ld	e,(ix+WRKAREA.PCTBUFF+3)
	ld	(iy+3),e			; Set the number of sectors
	ld	(iy+4),d
	ld	(iy+5),l
	ld	(iy+6),h

	ld	h,(ix+WRKAREA.PCTBUFF+4)	; Get the sector size
	ld	l,(ix+WRKAREA.PCTBUFF+5)
	ld	d,(ix+WRKAREA.PCTBUFF+6)
	ld	e,(ix+WRKAREA.PCTBUFF+7)

	ld	a,h	;If high word not zero, set zero (info not available)
	or	l
	ld	hl,0
	ex	de,hl
	jr	nz,.info_ssizeatapi

	ex	de,hl
	ld	a,d
	or	e
	jr	nz,.info_ssizeatapi
	ld	de,512	;If low word is zero, assume 512 bytes
.info_ssizeatapi:
	ld	(iy+1),e			; Set the sector size
	ld	(iy+2),d
	ld	(ix+DEVINFO.SECTSIZE),e
	ld	(ix+DEVINFO.SECTSIZE+1),d

	call	IDE_OFF
	xor	a
	ret

.errorpop:
	pop	hl

LUN_INFO_ERROR:
	call	IDE_OFF
	ld	a,1
	ret


;-----------------------------------------------------------------------------
;
; Physical format a device
;
;Input:   A = Device index, 1 to 7
;         B = Logical unit number, 1 to 7
;         C = Format choice, 0 to return choice string
;Output:
;        When C=0 at input:
;        A = 0: Ok, address of choice string returned
;            _IFORM: Invalid device or logical unit number,
;                    or device not formattable
;        HL = Address of format choice string (in bank 0 or 3),
;             only if A=0 returned.
;             Zero, if only one choice is available.
;
;        When C<>0 at input:
;        A = 0: Ok, device formatted
;            Other: error code, same as DEV_RW plus:
;            _IPARM: Invalid format choice
;            _IFORM: Invalid device or logical unit number,
;                    or device not formattable
;        B = Media ID if the device is a floppy disk, zero otherwise
;            (only if A=0 is returned)
;
; Media IDs are:
; F0h: 3.5" Double Sided, 80 tracks per side, 18 sectors per track (1.44MB)
; F8h: 3.5" Single sided, 80 tracks per side, 9 sectors per track (360K)
; F9h: 3.5" Double sided, 80 tracks per side, 9 sectors per track (720K)
; FAh: 5.25" Single sided, 80 tracks per side, 8 sectors per track (320K)
; FBh: 3.5" Double sided, 80 tracks per side, 8 sectors per track (640K)
; FCh: 5.25" Single sided, 40 tracks per side, 9 sectors per track (180K)
; FDh: 5.25" Double sided, 40 tracks per side, 9 sectors per track (360K)
; FEh: 5.25" Single sided, 40 tracks per side, 8 sectors per track (160K)
; FFh: 5.25" Double sided, 40 tracks per side, 8 sectors per track (320K)

DEV_FORMAT:
	ld	a,_IFORM
	ret


;-----------------------------------------------------------------------------
;
; Execute direct command on a device
;
;Input:    A = Device number, 1 to 7
;          B = Logical unit number, 1 to 7 (if applicable)
;          HL = Address of input buffer
;          DE = Address of output buffer, 0 if not necessary
;Output:   Output buffer appropriately filled (if applicable)
;          A = Error code:
;              0: Ok
;              1: Invalid device number or logical unit number,
;                 or device not ready
;              2: Invalid or unknown command
;              3: Insufficient output buffer space
;              4-15: Reserved
;              16-255: Device specific error codes
;
; The first two bytes of the input and output buffers must contain the size
; of the buffer, not incuding the size bytes themselves.
; For example, if 16 bytes are needed for a buffer, then 18 bytes must
; be allocated, and the first two bytes of the buffer must be 16, 0.

DEV_CMD:
	ld	a,2
	ret

;=====
;=====  END of DEVICE-BASED specific routines
;=====


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
; Wait the BSY flag to clear
; Note: This version has a short timeout and is intended for the normal r/w
; operations. It's not adequate to be used for slower commands like soft-reset
; or DIAGNOSTICS.
;
; Input:  Nothing
; Output: Cy=1 if timeout
;	  A = Contents of the status register

WAIT_BSY:
	out	(#E6),a			; Reset System-timer
	in	a,(#E7)
	or	a			; Is the timer present?
	jr	z,.hasTimer

	push	bc
	ld	bc,#0090		; 256 fast retries, 48 slow retries
.wait1:
	ld	a,(IDE_STATUS)
	and	M_BSY			; Still busy?
	jr	z,.end			; No, skip
	ex	(sp),hl
	ex	(sp),hl
	djnz	.wait1			; fast retry
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	dec	c
	jr	nz,.wait1		; slow retry
	scf				; Timeout: quit with Cy=on
.end:	pop	bc
	ret

.hasTimer:
	push	bc
	ld	b,1*4			; 1s
.twait1:
	ld	a,(IDE_STATUS)
	and	M_BSY
	jr	z,.tend			; Yes, quit
	in	a,(#E7)
	cp	250			; 250ms
	jr	c,.twait1
	out	(#E6),a			; Reset the timer again
	djnz	.twait1
	scf				; Timeout: quit with Cy=on
.tend:
	ld	a,(IDE_STATUS)		; Clear the INTRQ
	pop	bc
	ret





;-----------------------------------------------------------------------------
; Wait the BSY flag to clear and RDY flag to be set
; if we wait for more than 30s, send a soft reset to IDE BUS
; if the soft reset didn't work after 30s return with error
;
; Input:  Nothing
; Output: Cy=1 if timeout after soft reset 
;	  A = Contents of the status register

WAIT_DRDY:
	out	(#E6),a			; Reset System-timer
	in	a,(#E7)
	or	a			; Is the timer present?
	jr	z,.hasTimer

	push	bc
	ld	bc,#0000		; 256 fast retries, 256 slow retries
.wait1: ld	a,(IDE_STATUS)
	and	M_BSY+M_DRDY
	cp	M_DRDY			; BSY=0 and DRDY=1?
	jr	z,.end			; Yes, quit
	ex	(sp),hl
	ex	(sp),hl
	djnz	.wait1			; fast retry
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	dec	c
	jr	nz,.wait1		; slow retry
	scf
.end:	pop	bc
	ld	a,(IDE_STATUS)		; Clear the INTRQ
	ret

.hasTimer:
	push	bc
	ld	b,10*4			; 10s
.twait1:
	ld	a,(IDE_STATUS)
	and	M_BSY+M_DRDY
	cp	M_DRDY			; BSY=0 and DRDY=1?
	jr	z,.tend			; Yes, quit
	in	a,(#E7)
	cp	250			; 250ms
	jr	c,.twait1
	out	(#E6),a			; Reset the timer again
	djnz	.twait1
	scf				; Timeout: quit with Cy=on
.tend:	ld	a,(IDE_STATUS)
	pop	bc
	ret

;-----------------------------------------------------------------------------
;--- Check for device fault or error
;    Output: Cy=on and A=_DISK on fault

CHK_RW_FAULT:
	call	WAIT_BSY		; wait until the flags are valid
;	jr	c,.error		; quit on timeout
	ret	c			; quit on timeout

	ld	a,(IDE_STATUS)
	and	M_DF+M_ERR		; Device fault or error?
	ret	z			; No, quit wit Cy off
;.error:
;	call	IDE_OFF
;	ld	a,_DISK
;	ld	b,0
	scf
	ret

;-----------------------------------------------------------------------------
; Check for ERROR
; 
CHK_ERR:
	call	WAIT_BSY		; wait until the flags are valid
	ret	c
	bit	ERR,a			; error?
	ret	z			; No, quit
	scf
	ret


;-----------------------------------------------------------------------------
; Execute a PIO command
;
; Input:  A = Command code
;         Other command registers appropriately set
; Output: Cy=1 if ERR bit in status register set
;	  A = Contents of the status register

PIO_CMD:
	push	bc
	ld	c,a
	call	WAIT_BSY		; wait until the flags are valid
	ld	a,c
	pop	bc
	ret	c
	ld	(IDE_CMD),a
	; Must wait 400ns
	nop	; 1400ns@3.57MHz

;-----------------------------------------------------------------------------
; 
WAIT_DRQ:
	call	WAIT_BSY		; wait until the flags are valid
	ret	c			; quit on timeout

	out	(#E6),a			; Reset System-timer
	in	a,(#E7)
	or	a			; Is the timer present?
	jr	z,.hasTimer

	push	bc
	ld	bc,#0090		; 256 fast retries, 144 slow retries
.wait1: ld	a,(IDE_STATUS)
	rrca				; ERR=1?
	jr	c,.end2			; Yes, abort with error
	and	(M_DRQ>>1)		; DRQ=1?
	jr	nz,.end			; Yes, quit
	ex	(sp),hl
	ex	(sp),hl
	djnz	.wait1			; fast retry
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	ex	(sp),hl
	dec	c
	jr	nz,.wait1		; slow retry
	scf				; Cy = timeout
.end:
;	ex	(sp),hl			; Blind wait for the INTRQ, since
;	ex	(sp),hl			; this IDE interface doesn't allow
;	ex	(sp),hl			; us to know when the interrupt would
;	ex	(sp),hl			; be triggered.
.end2:	pop	bc
	ld	a,(IDE_STATUS)
	ret

.hasTimer:
	push	bc
	ld	b,5*4			; 5s
.twait1:
	ld	a,(IDE_STATUS)
	rrca				; error?
	jr	c,.tend2		; Yes, abort with Cy=on
	and	(M_DRQ>>1)		; DRQ=1?
	jr	nz,.tend		; Yes, quit
	in	a,(#E7)
	cp	250			; 250ms
	jr	c,.twait1
	out	(#E6),a			; Reset the timer again
	djnz	.twait1
	scf				; Timeout: quit with Cy=on
	jr	.tend2
.tend:
	out	(#E6),a			; Reset the timer
.tblindwait:				; Blind wait for the INTRQ, since
	in	a,(#E6)			; this IDE interface doesn't allow
					; us to know when the interrupt would
					; be triggered. Duh!
	cp	3			; 3*4us
	jr	c,.tblindwait
.tend2:	pop	bc
	ld	a,(IDE_STATUS)		; Clear the INTRQ
	ret





;-----------------------------------------------------------------------------
; Select a device 
; 
; This operation seems to require a delay, otherwise the devices may behave
; erratically
SELDEV:
	ex	af,af'
	call	WAIT_BSY
	ret	c
	ex	af,af'
	ld	(IDE_HEAD),a
	; Detect the system timer
	out	(#E6),a
	in	a,(#E7)
	or	a
	jr	z,.twait1
	ex	(sp),hl
	ex	(sp),hl
	ret
.twait1:
	in 	a,(#E6)
	cp	3			; 3*4us
	jr	c,.twait1
	ret

;-----------------------------------------------------------------------------
; Disable the interrupts for the current device
; 
; This operation seems to require a delay, otherwise the devices may behave
; erratically
DISDEVINT:
        ld	a,M_nIEN		; Disable interrupts
        ld      (IDE_DEVCTRL),a
	; Detect the system timer
	out	(#E6),a
	in	a,(#E7)
	or	a
	jr	z,.twait1
	ex	(sp),hl
	ex	(sp),hl
	ret
.twait1:
	in 	a,(#E6)
	cp	3			; 3*4us
	jr	c,.twait1
	ret

	ld	a,(IDE_STATUS)
	rrca				; ERR=1?
	ccf
	ret	nc			; Yes, quit
	jp	WAIT_BSY


;-----------------------------------------------------------------------------
;
; Do a soft reset on the IDE devices
;
; Input   : none
; Output  : Cy if timed out 
;           (INTFLG)=3 if the user pressed CTRL+STOP

RESET_ALL:
	ld	a,M_DEV			; Select SLAVE
	call	SELDEV
	call	WAIT_BSY
	xor	a			; Select MASTER
	call	SELDEV
	call	WAIT_BSY

	call	.atapirst

.ataonly:
	xor	a			; Select MASTER
	call	SELDEV
	call	WAIT_BSY

	out	(#E6),a
	in	a,(#E7)
	or	a			; Is the system-timer present?
	jr	z,.hasSystimer		; Yes, use it

.noSystimer:
        ld      a,M_SRST+M_nIEN		; Do a software reset
        ld      (IDE_DEVCTRL),a
	halt
	halt				; 16.6ms (spec: 5us)
        ld	a,M_nIEN		; stop reset
        ld      (IDE_DEVCTRL),a
	halt				; 16.6ms (spec: 2ms)
	call	WAIT_RST		; Wait for the resets to finish
	ret	c
	halt
	halt
	ret

.hasSystimer:
.twait1:
	in 	a,(#E6)
	cp	2			; 2*4us (spec: > 5us)
	jr	c,.twait1

        ld	a,M_nIEN		; stop reset
        ld      (IDE_DEVCTRL),a
	out	(#E6),a
.twait2:
	in 	a,(#E7)
	cp	3			; 3ms (spec: > 2ms)
	jr	c,.twait2
	call	WAIT_RST		; Wait for the resets to finish
	ret	c
	halt
	halt
	ret

.atapirst:
	; Reset a slave ATAPI
	ld	a,M_DEV			; Select SLAVE
	call	SELDEV
	ld	a,ATAPICMD.RESET
	ld	(IDE_CMD),a
	; Reset a master ATAPI
	xor	a			; Select MASTER
	call	SELDEV
	ld	a,ATAPICMD.RESET
	ld	(IDE_CMD),a
	ret


;-----------------------------------------------------------------------------
; Checks for a diagnostic error and set a warning flag accordingly
; Input : none
; Output: WRKAREA.MASTER.t321D and WRKAREA.SLAVE.t321D:
;	  0=no error
;         bit7=1: error.  b6~b0: reported error code
; Modifies: B


CHKDIAG:
	ld	a,(ix+WRKAREA.MASTER.t321D)
	inc	a			; Undetected master?
	scf
	ret	z			; Yes, quit with error
	ld	a,(IDE_ERROR)
	ld	b,a			; b=DIAG status
	and	#7F			; Crop the master error code
	cp	1			; Any error?
	jr	z,.chkslave		; No, skip
.saveerrms:	; Save the error code from the master
	or	#80
	ld	(ix+WRKAREA.MASTER.t321D),a
	bit	7,b			; Error on slave?
	scf
	ret	z			; No, quit
.chkslave:
	bit	7,b			; Error on slave?
	ret	z			; No, quit
	ld	a,M_DEV			; Select the slave
	call	SELDEV
	ld	a,(IDE_ERROR)
	and	#7F			; Crop the slave error code
	cp	1			; Any error?
	ret	z			; No, quit
.saveerrsl:	; Save the error code from the slave
	or	#80
	ld	(ix+WRKAREA.SLAVE.t321D),a
	ld	a,b
	cp	1			; Any error reported?
	ret	z			; No, quit with Cy=off
	scf				; Cy=on if there was any error
	ret

;-----------------------------------------------------------------------------
; Prints an explanation message for a diagnostic error
; Input: A=Diagnostic error

DIAGERRPRT:
	cp	#FE			; Undetected master?
	ld	de,DIAGS_S.nomaster
	jr	z,.print		; Yes, print
	and	#7F			; Crop the error code
	jr	z,.print		; 0=Undetected master: print
	ld	b,a
	dec	b			; adjust for the switch case
	djnz	.buff
	ld	de,DIAGS_S.formatter		; ERR=2
	jr	.print
.buff:	djnz	.ecc
	ld	de,DIAGS_S.buffer		; ERR=3
	jr	.print
.ecc:	djnz	.mcu
	ld	de,DIAGS_S.ECC			; ERR=4
	jr	.print
.mcu:	djnz	.unkn
	ld	de,DIAGS_S.microcontroller	; ERR=5
.print:	jp	PRINT
.unkn:	ld	de,DIAGS_S.unknown
	push	af
	call	PRINT
	pop	af
	call	PRINTHEXBYTE
	ld	a,'>'
	call	CHPUT
	ld	de,CRLF_S
	jr	.print

;-----------------------------------------------------------------------------
; Subroutine to read blocks of arbitrary size on the IDE 
; Input: DE =Data destination
;	 BC =block size
;        IYL=Number of blocks
READ_DATA:
	call	SETLDIRHLPR	; hl'=Pointer to data transfer helper
.loop:
	call	WAIT_DRQ
	ret	c
	ld	hl,IDE_DATA
	call	RUN_HLPR
	dec	iyl
	jp	nz,.loop
	ret


;-----------------------------------------------------------------------------
; Subroutine to write blocks of arbitrary size on the IDE 
; Input: HL =Data source
;        BC =block size
;        IYL=Number of blocks
WRITE_DATA:
	call	SETLDIRHLPR	; hl'=Pointer to LDIR helper in RAM
.loop:
	call	WAIT_DRQ
	ret	c
	ld	de,IDE_DATA
	call	RUN_HLPR
	call	CHK_RW_FAULT
	ret	c
	dec	iyl
	jp	nz,.loop
	ret

LDI512:	; Z80 optimized 512 byte transfer
	exx
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
	ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
    ldi
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
; Obtain the work area address for the driver or the device
; Input: A=Selects where to point inside the work area
;	   0: base work area
;	   1: work area for the master
;	   2: work area for the slave
; Output: IX=Pointer to the selected work area
; Modifies: disables the IDE registers

MY_GWORK:
	push	af
	xor	a
	EX	AF,AF'
	XOR	A
	LD	IX,GWORK
	call	CALBNK
	pop	af
	push	de
	ld	e,(ix)			; de=Pointer to the WorkAREA in RAM 
	ld	d,(ix+1)
	ld	ix,0
	or	a
	jr	z,.end
	cp	1
	ld	ix,WRKAREA.MASTER.BASE
	jr	z,.end
	ld	ix,WRKAREA.SLAVE.BASE
.end:	add	ix,de			; Point ix to the device work area
	pop	de
	ret

;-----------------------------------------------------------------------------
;
; Check the device index and LUN
; Input:  A = device index, B = lun
; Output: Cy=0 if OK, 1 if device or LUN invalid
;         IX = Work area for the device
; Modifies F, C

CHECK_DEV_LUN:
	or	a	;Check device index
	scf
	ret	z	;Return with error if devindex=0
	cp	3
	ccf
	ret	c	;Return with error if devindex>2

	ld	c,a	; c=device index



	ld	a,b	; Check LUN number
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
	ld	a,(ix+DEVINFO.t321D)
	and	3
	jr	z,.nodev
	cp	1		; TODO: Implement CHS support
	jr	z,.nodev	; For now, CHS devices are unsupported
	ld	a,c
	or	a
	ret

.nodev:
	ld	a,c
	scf
	ret



; ------------------------------------------------
; Jumps to a helper routine, usually in RAM
; Input: HL': Address of the target routine
; ------------------------------------------------
RUN_HLPR:
	exx
	jp	(hl)

; ------------------------------------------------
; Setup the arbitrary block size LDIR helper to be used
; Input   : BC : block size  (must be >2 bytes)
; Output  : HL': Address of the LDIR helper in RAM
; Modifies: AF, DE', HL'
; ------------------------------------------------
SETLDIRHLPR:
	ld	a,b
	or	c		; Shortcut comparison. Takes advantage that
				; only 512 and 2 will have this OR bitmask
				; to save time
	cp	HIGH 512	; bc=512?
	exx
	jr	nz,.useLDIR	; No, must use LDIR then

	; Check for a Z80 or R800
	xor	a		; Clear Cy
	dec	a		; A=#FF
	db	#ED,#F9		; mulub a,a
	jr	c,.useLDIR	; Always use LDIR in RAM for the R800

	ld	hl,LDI512
	exx
	ret

.useLDIR:
	exx
	push	bc
	exx
	pop	de		; de=block size
	ld	l,(ix+DEVINFO.pBASEWRK)
	ld	h,(ix+DEVINFO.pBASEWRK+1)
	; Set the the block size
	; *** BLKLEN must be the first data in the workArea
	ld	(hl),e
	inc	hl
	ld	(hl),d
	; Point to the data transfer routine
	ld	de,WRKAREA.LDIRHLPR-1
	add	hl,de
	exx			; hl'=Pointer to LDIR helper routine
	ret



; ------------------------------------------------
; Initialize the Work-area
; ------------------------------------------------
INIWORK:
	; Clear the WorkArea
	push	ix
	pop	hl
	push	hl
	ld	b,WRKAREA
	xor	a
.clrwork2:
	ld	(hl),a
	inc	hl
	djnz	.clrwork2

	pop	hl
	; Set the pointers to go back to the base of the WorkArea
	ld	(ix+WRKAREA.MASTER.pBASEWRK),l
	ld	(ix+WRKAREA.MASTER.pBASEWRK+1),h
	ld	(ix+WRKAREA.SLAVE.pBASEWRK),l
	ld	(ix+WRKAREA.SLAVE.pBASEWRK+1),h

	; Install the data transfer helper routine in the WorkArea 
	; This speeds up the LDIR speed a lot for the R800
	ld	de,WRKAREA.LDIRHLPR
	add	hl,de
	ex	de,hl
	ld	hl,R800DATHLP
	ld	bc,R800DATHLP.end-R800DATHLP
	ldir

	; Point the LD BC (addr) from LDIRHLPR to BLKLEN
	push	ix
	pop	hl
	ld	de,WRKAREA.BLKLEN
	add	hl,de				; hl=pointer to BLKLEN
	ld	(ix+WRKAREA.LDIRHLPR+3),l
	ld	(ix+WRKAREA.LDIRHLPR+4),h
	ret

; ------------------------------------------------
; R800 data transfer routine, copied to the WorkArea
; ------------------------------------------------
R800DATHLP:
	exx
	ld	bc,(0)		; The address will be set by INIWORK
	ldir
	ret
.end:

;-----------------------------------------------------------------------------
;
; Restore screen parameters on MSX>=2 if they're not set yet
; Input   : none
; Output  : none
; Modifies: all
MYSETSCR:
	ld	a,(MSXVER)
	or	a			; MSX1?
	jr	nz,.notMSX1		; No, skip
.MSX1:
	ld	a,(SCRMOD)
	or	a			; SCREEN0 already?
	ret	z			; Yes, quit
	jp	INITXT			; set screen0

.notMSX1:
	ld	c,$23			; Block-2, R#3
	ld 	ix,REDCLK
	call	EXTROM
	and	1
	ld	b,a
	ld	a,(SCRMOD)
	cp	b
	jr	nz,.restore
	inc	c
	ld 	ix,REDCLK
	call	EXTROM
	ld	b,a
	inc	c
	ld 	ix,REDCLK
	call	EXTROM
	add	a,a
	add	a,a
	add	a,a
	add	a,a
	or	b
	ld	b,a
	ld	a,(LINLEN)
	cp	b
	ret	z
.restore:
	xor	a		; Don't displat the function keys
	ld	ix,SDFSCR
	jp	EXTROM

;-----------------------------------------------------------------------------
; Prints a byte in hex
; Input : A=byte to be printed
; Modifies: C
PRINTHEXBYTE:
	ld	c,a
	rrca
	rrca
	rrca
	rrca
	and	#F
	call	printnibble
	ld	a,c
	and	#F
	call	printnibble
	ret

printnibble:
	cp	10		; <=9?
	jr	c,.print09	; Yes, skip
;	ld	a,9		; Limit to 1 digit
	cp	15
	jr	c,.printAF
	ld	a,#F		; Limit to 15
.printAF:
	add	"A"-10
	jp	CHPUT
.print09:
	add	"0"
	jp	CHPUT

;-----------------------------------------------------------------------------
;
; Check if the STOP key was signaled on DRV_INIT
; Input   : none
; Output  : none
; Modifies: all

INICHKSTOP:
	ld	a,(INTFLG)
	cp	4			; Was STOP pressed?
	ret	nz			; No, quit as fast as possible 

	; Handle STOP to pause and read messages, and ask for the copyright info
	ld	de,BOOTPAUSE_S
	call	PRINT
.wait1:	ld	a,7
	call	SNSMAT
	and	$10			; Is STOP still pressed?
	jr	z,.wait1		; Wait for STOP to be released
	xor	a
	ld	(INTFLG),a		; Clear STOP flag
	ld	b,0			; b=inhibit 'i' key flag
.wait2: call	CHSNS
	call	nz,.chkikey		; Wait until a key is pressed
	ld	a,(INTFLG)
	cp	4			; Was STOP pressed?
	jr	nz,.wait2		; No, return
	xor	a
	ld	(INTFLG),a		; Clear STOP flag
	call	KILBUF
	ld	b,30			; Since the user is trying pause the
.wait3:	halt				; boot messages, this gives him enough
					; time to react and pause the next
					; driver
	ld	a,(INTFLG)
	cp	4			; Was STOP pressed?
	ret	z			; quit so the next driver can process it
	djnz	.wait3			; The user will have the impression
					; that he has a perfect timing.   ;)
	ret

.chkikey:
	bit	0,b			; Was the copyright message shown?
	ret	nz			; Yes, return
	call	CHGET
	cp	'i'
	jr	z,.showcopyright
	cp	'I'
	ret	nz
.showcopyright:
	inc	b			; Inhibit further presses of the i key 
	ld	de,COPYRIGHT_S
	jp	PRINT





;=======================
; Strings
;=======================

INFO_S:
	db	13,"Sunrise compatible IDE driver v",27,'J'
	db	VER_MAIN+$30,'.',VER_SEC+$30,'.',VER_REV+$30
CRLF_S:	db	13,10,0
COPYRIGHT_S:
	db	"(c) 2009 Konamiman",13,10
	db	"(c) 2014 Piter Punk",13,10
	db	"(c) 2017 FRS",13,10,13,10,0

BOOTPAUSE_S:
	db	"Paused. Press <i> to show the copyright info.",13,10,0

SEARCH_S:
	db	"Searching: ",0

NODEVS_S:
	db	"not found",13,10,0
ABORTED_S:
	db	"<aborted>",13,10,0
INIT_S:
	db	13,"Initializing : ",27,'J',0
MASTER_S:
	db	13,"Master device: ",27,'J',0
SLAVE_S:
	db	13,"Slave device : ",27,'J',0

OK_S:	db	"Ok",13,10,0
ERROR_S:
	db	"Error!",13,10,0

DETECT_S:
	db	"detecting",0
.unknown:	db	"Unknown device ",0
.unsupported:	db	"Unsuppored device",0
.oparenthesis:	db	" (",0
.chs:		db	"CHS",0
.lba:		db	"LBA",0
.atapi:		db	"ATP",0


DIAGS_S:
.nomaster:		db	"<failed>",7,13,10,0
.formatter:		db	"<formatter device error>",7,13,10,0
.buffer:		db	"<sector buffer error>",7,13,10,0
.ECC:			db	"<ECC circuitry error>",7,13,10,0
.microcontroller:	db	"<controlling microprocessor error>",7,13,10,0
.unknown:		db	"<unknown error ",7,0


;=======================
; Variables
;=======================
	.phase	#C000
TEMP_WORK	WRKTEMP
	.dephase



;-----------------------------------------------------------------------------
;
; Padding up to the required driver size

DRV_END:

	ds	3ED0h-(DRV_END-DRV_START)

	end
