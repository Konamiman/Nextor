;-----------------------------------------------------------------------------
;
; MegaFlashROM SCC+ SD disk driver for Nextor
; By Manuel Pazos 14/12/2012
;
; 24/07/2018 - v1.3 Implement DRV_CONFIG routine (Nextor 2.0.5)
;-----------------------------------------------------------------------------

DRIVER_VERSION		equ	1
DRIVER_SUBVERSION	equ	3

	.RELAB

;---------------------------------------------------------------------------
; MACROS
;---------------------------------------------------------------------------

ADD_HL_A macro
		add	a, l	; 4
		ld	l, a	; 4
		adc	a, h	; 4
		sub	l	; 4
		ld	h, a	; 4
	ENDM

ADD_DE_A macro
		add	a, e	; 4
		ld	e, a	; 4
		adc	a, d	; 4
		sub	e	; 4
		ld	d, a	; 4
	ENDM

;-----------------------------------------------------------------------------
; Constants
;-----------------------------------------------------------------------------

WORK_AREA_SIZE	equ	0	; RAM needed by the driver

	;The NUM_SLOTS constant (SD card slots available) must be defined externally

	ifndef NUM_SLOTS
	.fatal NUM_SLOTS constant required with a value of 1 or 2
	else
	if (NUM_SLOTS NE 1) and (NUM_SLOTS NE 2)
	.fatal The value of NUM_SLOTS must be either 1 or 2, got {NUM_SLOTS}
	endif
	endif

; Work area offsets
STATUS		equ	0	; Offset in work area to status byte

BIT_SDHC	equ	0	; 0 = SDSC, 1 = SDHC
BIT_SD_CHG	equ	1	; 0 = Not changed, 1 = Changed
BIT_ROM_DSK	equ	2	; 0 = Not available, 1 = Available
;CARD_TYPE	equ	31

; Cards types
CARD_MMC	equ	0
CARD_SD1X	equ	1
CARD_SD2X	equ	2
CARD_SDHC	equ	3


;This is a 2 byte buffer to store the address of code to be executed.
;It is used by some of the kernel page 0 routines.
CODE_ADD:	equ	0F2EDh

; BIOS
ENASLT:		equ	#24
INITXT		equ	#6C
CHGET		equ	#9f
CHPUT		equ	#A2	;Character output
RSLREG:		equ	#138
EXTROM		equ	#15F

SDFSCR		equ	#185
REDCLK		equ	#1F5

MSXVER		equ	#2D
LINL40		equ	#F3AE	; Width
LINLEN		equ	#F3B0
BDRCLR		equ	#F3EB
SCRMOD		equ	#FCAF
EXPTBL:		equ	#FCC1
SLTWRK:		equ	#FD09

; SD
DATA_TOKEN	equ	#FE
MUL_DAT_TKN_STA	equ	#FC
MUL_DAT_TKN_END	equ	#FD

;Driver type:
DRV_TYPE	equ	1	; 0 for drive-based, 1 for device-based

;Driver version
VER_MAIN	equ	1
VER_SEC		equ	3
VER_REV		equ	0


; Error codes for DEV_RW
    IF DRV_TYPE = 1
NCOMP		equ	0FFh
WRERR		equ	0FEh
DISK		equ	0FDh
NRDY		equ	0FCh
DATA		equ	0FAh
RNF		equ	0F9h
WPROT		equ	0F8h
UFORM		equ	0F7h
SEEK		equ	0F3h
IFORM		equ	0F0h
IDEVL		equ	0B5h
IPARM		equ	08Bh
    ENDIF


;-----------------------------------------------------------------------------
; Start
;-----------------------------------------------------------------------------

	org	#4000

	ds	256,0	; Dummy bytes. Will be overwriten by Page 0 code

DRV_START:

;-----------------------------------------------------------------------------
;
; Routines and information available on kernel page 0
;
;-----------------------------------------------------------------------------
;* Get in A the current slot for page 1. Corrupts F.
;  Must be called by using CALBNK to bank 0:
;    xor a
;    ld ix,GSLOT1
;    call CALBNK

GSLOT1		equ	402Dh


;* This routine reads a byte from another bank.
;  Must be called by using CALBNK to the desired bank,
;  passing the address to be read in HL:
;    ld a,<bank number>
;    ld hl,<byte address>
;    ld ix,RDBANK
;    call CALBNK

RDBANK		equ	403Ch


;* This routine temporarily switches kernel main bank
;  (usually bank 0, but will be 3 when running in MSX-DOS 1 mode),
;  then invokes the routine whose address is at (CODE_ADD).
;  It is necessary to use this routine to invoke CALBAS
;  (so that kernel bank is correct in case of BASIC error)
;  and to invoke DOS functions via F37Dh hook.
;
;  Input:  Address of code to invoke in (CODE_ADD).
;          AF, BC, DE, HL, IX, IY passed to the called routine.
;  Output: AF, BC, DE, HL, IX, IY returned from the called routine.

CALLB0		equ	403Fh


;* Call a routine in another bank.
;  Must be used     IF the driver spawns across more than one bank.
;
;  Input:  A = bank number
;          IX = routine address
;          AF' = AF for the routine
;          HL' = Ix for the routine
;          BC, DE, HL, IY = input for the routine
;  Output: AF, BC, DE, HL, IX, IY returned from the called routine.

CALBNK		equ	4042h


;* Get in IX the address of the SLTWRK entry for the slot passed in A,
;  which will in turn contain a pointer to the allocated page 3
;  work area for that slot (0     IF no work area was allocated).
;      IF A=0, then it uses the slot currently switched in page 1.
;  Returns A=current slot for page 1,     IF A=0 was passed.
;  Corrupts F.
;  Must be called by using CALBNK to bank 0:
;    ld a,<slot number> (xor a for current page 1 slot)
;    ex af,af'
;    xor a
;    ld ix,GWORK
;    call CALBNK

GWORK		equ	4045h


;* This address contains one byte that tells how many banks
;  form the Nextor kernel (or alternatively, the first bank
;  number of the driver).

K_SIZE		equ	40FEh


;* This address contains one byte with the current bank number.

CUR_BANK	equ	40FFh


;-----------------------------------------------------------------------------
;
; Built-in format choice strings
;
;-----------------------------------------------------------------------------
NULL_MSG  equ     781Fh	;Null string (disk can't be formatted)
SING_DBL  equ     7820h ;"1-Single side / 2-Double side"


;-----------------------------------------------------------------------------
;
; Driver signature
;
;-----------------------------------------------------------------------------
	db	"NEXTOR_DRIVER",0


;-----------------------------------------------------------------------------
;
; Driver flags:
;    bit 0: 0 for drive-based, 1 for device-based
;    bit 1: 1 for hot-plug devices supported (device-based drivers only)
;    bit 2: 1 if the driver provides configuration
;             (implements the DRV_CONFIG routine)
;-----------------------------------------------------------------------------
    IF DRV_TYPE = 0
	db	0
    ENDIF

    IF DRV_TYPE = 1
	db	%00000111
    ENDIF


;-----------------------------------------------------------------------------
;
; Reserved byte
;
;-----------------------------------------------------------------------------
	db	0


;-----------------------------------------------------------------------------
;
; Driver name
;
;-----------------------------------------------------------------------------
DRV_NAME:
	db	"MegaFlashROM SCC+ SD"
	ds	32-($-DRV_NAME)," "


;-----------------------------------------------------------------------------
;
; Jump table for the driver public routines
;
;-----------------------------------------------------------------------------	
	; These routines are mandatory for all drivers
        ; (but probably you need to implement only DRV_INIT)

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
        jp	DRV_CONFIG

	ds	12

    IF DRV_TYPE = 0

	; These routines are mandatory for drive-based drivers

        jp      DRV_DSKIO
        jp      DRV_DSKCHG
        jp      DRV_GETDPB
        jp      DRV_CHOICE
        jp      DRV_DSKFMT
        jp      DRV_MTOFF
    ENDIF

    IF DRV_TYPE = 1

	; These routines are mandatory for device-based drivers

	jp	DEV_RW
	jp	DEV_INFO
	jp	DEV_STATUS
	jp	LUN_INFO
	jp	DEV_FORMAT
	jp	DEV_CMD
    ENDIF


;=====
;=====  END of data that must be at fixed addresses
;=====

;-----------------------------------------------------------------------------
;
; BASIC expanded statement ("CALL") handler.
; Works the expected way, except that     IF invoking CALBAS is needed,
; it must be done via the CALLB0 routine in kernel page 0.
;-----------------------------------------------------------------------------
DRV_BASSTAT:
	scf
	;include	"expcall.asm"
	ret
	
;-----------------------------------------------------------------------------
; Dejamos libre el area usada por los puertos del MegaSD #4000-#5FFF
;-----------------------------------------------------------------------------
	ds #6000-$


;-----------------------------------------------------------------------------
; Timer interrupt routine, it will be called on each timer interrupt
; (at 50 or 60Hz), but only     IF DRV_INIT returns Cy=1 on its first execution.
;-----------------------------------------------------------------------------
DRV_TIMI:
	ret


;-----------------------------------------------------------------------------
;
; Driver initialization routine, it is called twice:
;
; 1) First execution, for information gathering.
;    Input:
;      A = 0
;      B = number of available drives
;      HL = maximum size of allocatable work area in page 3
;    Output:
;      A = number of required drives (for drive-based driver only)
;      HL = size of required work area in page 3
;      Cy = 1     IF DRV_TIMI must be hooked to the timer interrupt, 0 otherwise
;
; 2) Second execution, for work area and hardware initialization.
;    Input:
;      A = 1
;      B = number of allocated drives for this controller
;
;    The work area address can be obtained by using GWORK.
;
;        IF first execution requests more work area than available,
;    second execution will not be done and DRV_TIMI will not be hooked
;    to the timer interrupt.
;
;        IF first execution requests more drives than available,
;    as many drives as possible will be allocated, and the initialization
;    procedure will continue the normal way
;    (for drive-based drivers only. Device-based drivers always
;     get two allocated drives.)
;-----------------------------------------------------------------------------
DRV_INIT:
		or	a
		jr	nz,.scrnset
	
		ld	hl,WORK_AREA_SIZE
		;ld	a,NUM_DRIVES
		ret		;Note that Cy is 0 (no interrupt hooking needed)

.scrnset:
		call	MYSETSCR
		jr	nz,.secondExe	; Second execution

.secondExe:
		ld	de,TXT_INFO
		call	PRINT		; Driver info

		ld	c,0
		call	RomDiskCheck
		jr	nz,.notFound	; ROM disk not found

		ld	de,TXT_ROMDSKOK
		call	PRINT		; Print "Rom disk found"

		ld	c,1		; Flag ROM disk found
.notFound:		
		ld	b,NUM_SLOTS
.loop:	
		push	bc
		di
	
		ld	a,NUM_SLOTS
		sub	b
		call	GETWRK
		dec	c
		jr	nz,.notRomDisk
	
		set	BIT_ROM_DSK,(ix+0)	; ROM disk available
.notRomDisk:	

		call	SD_ON
		ld	a,NUM_SLOTS
		sub	b
		ld	c,a
		push	bc
		ld	(#5800),a	; Select SD slot
		call	InitSD		; Init SD card
		call	SD_OFF
		pop	bc			; C = SD slot
		ei

		.COMMENT *
		push	af
		ld a,e
		and #f
		add a,"0"
		call	CHPUT	; Error code
		pop	af
		;*

		jr	c,.notCard
		jr	nz,.notCard

		ld	b,e		; Card type
		ld	de,TXT_INIT
		call	PRINT	; Card init text
	
		ld	a,c			; SD slot
		add	a,'1'
		call	CHPUT
		ld	a,':'
		call	CHPUT
		
		ld	a,b		; Card type
		rlca
		ld	hl,IDX_TYPE
		ADD_HL_A
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		call	PRINT	; Shows card type
.notCard:
		pop	bc
		djnz	.loop
		
		ld	de,TXT_EMPTY
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
;-----------------------------------------------------------------------------
DRV_VERSION:
	ld	a,VER_MAIN
	ld	b,VER_SEC
	ld	c,VER_REV
	ret





;-----------------------------------------------------------------------------
;
; BASIC expanded device handler.
; Works the expected way, except that     IF invoking CALBAS is needed,
; it must be done via the CALLB0 routine in kernel page 0.
;-----------------------------------------------------------------------------
DRV_BASDEV:
	scf
	ret


;-----------------------------------------------------------------------------
;
; Extended BIOS hook.
; Works the expected way, except that it must return
; D'=1     IF the old hook must be called, D'=0 otherwise.
; It is entered with D'=1.
;-----------------------------------------------------------------------------
DRV_EXTBIO:
	ret


;-----------------------------------------------------------------------------
;
; Direct calls entry points.
; Calls to addresses 7450h, 7453h, 7456h, 7459h and 745Ch
; in kernel banks 0 and 3 will be redirected
; to DIRECT0/1/2/3/4 respectively.
; Receives all register data from the caller except IX and AF'.
;-----------------------------------------------------------------------------
DRV_DIRECT0:
DRV_DIRECT1:
DRV_DIRECT2:
DRV_DIRECT3:
DRV_DIRECT4:
	ret

;-----------------------------------------------------------------------------
;
; Get driver configuration
;
; Input:
;   A = Configuration index
;   BC, DE, HL = Depends on the configuration
;
; Output:
;   A = 0: Ok
;       1: Configuration not available for the supplied index
;   BC, DE, HL = Depends on the configuration
;
; * Get number of drives at boot time (for device-based drivers only):
;   Input:
;     A = 1
;     B = 0 for DOS 2 mode, 1 for DOS 1 mode
;   Output:
;     B = number of drives
;
; * Get default configuration for drive
;   Input:
;     A = 2
;     B = 0 for DOS 2 mode, 1 for DOS 1 mode
;     C = Relative drive number at boot time
;   Output:
;     B = Device index
;     C = LUN index

DRV_CONFIG:
	dec	a
	jr	z,.GetNumDrives
	
	dec	a
	jr	z,.GetRelDrvNum

.error:	
	ld	a,1			; Unknown configuration index
	ret
	
.GetNumDrives:
	bit 5,c         ;Single drive per driver requested?
    ld b,1
    ld a,0
    ret nz
	
	call	RomDiskCheck
	ld	b,NUM_SLOTS+1			; Three drives: ROM disk, SD 1 and SD 2
	jr	z,.GetNumDrives2

	dec	b			; ROM Disk not available
	
.GetNumDrives2:
	xor	a
	ret
	
.GetRelDrvNum:

	ld	b,c
	inc	b
	call	RomDiskCheck
	
	jr	z,.ok

	inc	b

.ok:
	ld	c,1
	xor	a
	ret


;=====
;=====  BEGIN of DRIVE-BASED specific routines
;=====

    IF DRV_TYPE = 0
	include	"drivebased.asm"
    ENDIF

;=====
;=====  END of DRIVE-BASED specific routines
;=====



;=====
;=====  BEGIN of DEVICE-BASED specific routines
;=====

    IF DRV_TYPE = 1

;-----------------------------------------------------------------------------
;
; Read or write logical sectors from/to a logical unit
;
;Input:    Cy=0 to read, 1 to write
;          A = Device number, 1 to 7
;          B = Number of sectors to read or write
;          C = Logical unit number, 1 to 7
;          HL = Source or destination memory address for the transfer
;          DE = Address where the 4 byte sector number is stored.
;Output:   A = Error code (the same codes of MSX-DOS are used):
;              0: Ok
;              .IDEVL: Invalid device or LUN
;              .NRDY: Not ready
;              .DISK: General unknown disk error
;              .DATA: CRC error when reading
;              .RNF: Sector not found
;              .UFORM: Unformatted disk
;              .WPROT: Write protected media, or read-only logical unit
;              .WRERR: Write error
;              .NCOMP: Incompatible disk.
;              .SEEK: Seek error.
;          B = Number of sectors actually read (in case of error only)
;-----------------------------------------------------------------------------
DEV_RW:
	
	push	af
	dec	a
	jp	z,.romdisk	; ROM disk device

	dec	a
	cp	NUM_SLOTS	; Device number
	jp	nc,.error
	
	dec	c		; LUN
	jp	nz,.error

	di
	call	SD_ON
	ld	(#5800),a	; SD slot select
	call	GETWRK

	pop	af		; Cy=0 to read, 1 to write
	
	jr	c,.write	; Write
	
	; Read
	;push	af
	;ld	a,#f6
	;call	Color
	;pop	af
	
	.COMMENT *
	;Count read sectors ----------------------------
	push	hl
	push	af
	ld	a,b
	ld	hl,(#f9f0)
	add	a, l	; 4
	ld	l, a	; 4
	adc	a, h	; 4
	sub	l	; 4
	ld	h, a	; 4
	ld	(#f9f0),hl
	pop	af
	pop	hl
	;----------------------------
	;*
	
	call	ReadSD
	
	;push	af
	;ld	a,#f0
	;call	Color
	;pop	af
	

	;ld	hl,(#f9f0)
	;inc	hl
	;ld	(#f9f0),hl	; Cuenta lecturas
		
	call	SD_OFF
	ei
	jr	nc,.ok
	
	ld	b,0
	;ld	a,DISK		; General unknown disk error
	ret
	
.ok:	
	xor	a		; Ok
	ret

.write:
	; Write
	call	WriteSD
	call	SD_OFF
	ei
	jr nc,.ok		; Ok

	.COMMENT *
	; DEBUG: Print error number in A
	ld	c,"0"
	add	a,c	
	ld	ix,#a2
	ld	iy,(#fcc1-1)
	call	#1c
	;*

.writeError:
	ld	b,0	
	ld	a,WRERR		; Write error
	ret			; Error
	
.error:
	pop	af
	ld	b,0
	ld	a,IDEVL		; Invalid device or LUN
	ret

;----------
; ROM disk
;----------
.romdisk:
	dec	c		; LUN
	jp	nz,.error

	pop	af
	jr	c,.writeError	; Can't write in ROM

	jp	RomDiskRead

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
;+0 (1): Numer of logical units, from 1 to 7. 1     IF the device has no logical
;        units (which is functionally equivalent to having only one).
;+1 (1): Device flags, always zero in Alpha 2b.
;
; The strings must be printable ASCII string (ASCII codes 32 to 126),
; left justified and padded with spaces. All the strings are optional,
;     IF not available, an error must be returned.
;     IF a string is provided by the device in binary format, it must be reported
; as an hexadecimal, upper-cased string, preceded by the prefix "0x".
; The maximum length for a string is 64 characters;
;     IF the string is actually longer, the leftmost 64 characters
; should be provided.
;
; In the case of the serial number string, the same rules for the strings
; apply, except that it must be provided right-justified,
; and     IF it is too long, the rightmost characters must be
; provided, not the leftmost.
;-----------------------------------------------------------------------------
DEV_INFO:
		dec	a
		jp	z,RomDiskInfo		; ROM disk
		
		dec	a
		cp	NUM_SLOTS
		jp	nc,.DEV_INFO_ERROR
		
		call	GETWRK
		push	ix
		pop	de
        	
		di
		call	SD_ON
		ld	(#5800),a	; SD slot select
        	
		push	bc
		push	hl
		call	GetCID
		pop	de		; DE = Buffer in RAM, HL = SD registers
		pop	bc
		jp	c,.DEV_INFO_ERROR
		
		djnz	.DEV_INFO2
		
		; 1: Manufacturer name string
        	
		ld	a,(hl)	; ID
		push	af	
		ld	b,15 + 2
		call	SkipBytes
		call	GetManufacName
		ld	b,0
		ldir		; Copy manufacturer name to buffer
		pop	bc
		
		; Add [id] to the manufacturer name
		ld	a,"["
		ld	(de),a
		inc	de
		ld	a,b
		call	Num2Hex.Num1
		ld	a,b
		call	Num2Hex.Num2
		ld	a,"]"
		ld	(de),a
		inc	de
		jr	.DEV_END
	
	
.DEV_INFO2:
		djnz	.DEV_INFO3
		
		; 2: Device name string
		ld	a,(hl)		; MID
		ld	a,(hl)		; OID byte 1
		ld	a,(hl)		; OID byte 2
		ld	bc,5
		ldir
		
		ld	b,8 + 2
		call	SkipBytes
		jr	.DEV_END
	
	
.DEV_INFO3:
		djnz	.DEV_INFO0
		
		; 3: Serial number string
		;ex	de,hl
		ld	b,9
		call	SkipBytes
        	
		call	.num
		call	.num
        	
		ld	b,3 + 2
		call	SkipBytes
		jr	.DEV_END
.num:	
		ld	a,(hl)
		ld	l,(hl)
		ld	h,a
		call	Num2Hex
		ld	h,#40
		ret

.DEV_INFO0:
		; 0: Basic information
		ld	b,16 + 2
		call	SkipBytes
		ex	de,hl
		ld	(hl),1	; Number of logical units
		inc	hl
		ld	(hl),0	; Flags: always zero in Alpha 2b.
		jr	.DEV_END2
		;xor	a
		;ret
	
.DEV_END:
		xor	a
		ld	(de),a
.DEV_END2:
		call	SD_OFF
		ei
		xor	a
		ret

.DEV_INFO_ERROR:	
		call	SD_OFF
		ei
		ld	a,1
		ret


;------------------------------------------------------------------------------
; Input: HL = number to convert, DE = location of ASCII string
; Output: ASCII string at (DE) 	
;------------------------------------------------------------------------------
Num2Hex:
		ld	a,h
		call	.Num1
		ld	a,h
		call	.Num2
		ld	a,l
		call	.Num1
		ld	a,l
		jr	.Num2

.Num1:
		rra
		rra
		rra
		rra
.Num2:
		or	#F0
		daa
		add	a,#A0
		adc	a,#40

		ld	(de),a
		inc	de
		ret
		
;-----------------------------------------------------------------------------
; Get manufacturer name
; In:	A = ID
; Out:	HL = String
;	BC = String length
;-----------------------------------------------------------------------------
GetManufacName:
		ld	c,a
		ld	hl,Manufacturers
.loop:
		ld	a,(hl)
		inc	hl
		cp	c
		jr	z,.found

		or	a
		jr	z,.found	; No more names in the list

		push	bc
		call	.found
		add	hl,bc
		inc	hl
		pop	bc
		jr	.loop		
.found:
		ld	c,0
		push	hl
		ld	a,"$"
.cont:
		inc	c
		inc	hl
		cp	(hl)
		jr	nz,.cont
.end:
		pop	hl
		ld	b,0		
		ret
				
Manufacturers:
		db	#01,"Panasonic$"
		db	#02,"Toshiba$"
		db	#03,"SanDisk$"
		db	#04,"(SMI-S?)"
		db	#06,"Renesas$"
		db	#11,"Dane-Elec$"
		db	#15,"Samsumg$"
		db	#18,"Infineon$"
		db	#13,"(KingMax?)$"
		db	#1a,"(PQI?)$"
		db	#1b,"(Sony?)$"
		db	#1c,"(Transcend?)$"
		db	#1d,"(A-DATA?)$"
		db	#27,"Verbatim$"
		db	#37,"(KINGMAX?)$"
		db	#41,"OKI$"
		db	#73,"SilverHT$"
		db	#aa,"openMSX$"
		db	#00,"Unknown$"
;-----------------------------------------------------------------------------
;
; Obtain device status
;
;Input:   A = Device index, 1 to 7
;         B = Logical unit number, 1 to 7
;             0 to return the status of the device itself.
;Output:  A = Status for the specified logical unit,
;             or for the whole device     IF 0 was specified:
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
;-----------------------------------------------------------------------------
DEV_STATUS:
		dec	a
		jp	z,RomDiskStatus		; ROM disk

		dec	a
		cp	NUM_SLOTS
		jp	nc,DEV_STAT1
		ld	c,a
		
		ld	a,b
		cp	2
		jp	nc,DEV_STAT1

		di
		ld	a,c
		call	GETWRK
		call	SD_ON
		ld	(#5800),a	; SD slot select

		;push	ix
		call	TestCard	;call	GetCID
		;pop	de		; DE = Work area, HL = SD registers

		call	SD_OFF
		ei
		
		jr	c,DEV_STAT0
		
		bit	BIT_SD_CHG,(ix+STATUS); Has been the card initiated?
		jr	z,.notChanged
	;ld a,3
	;ret
		res	BIT_SD_CHG,(ix+STATUS); Reset changed status

		;ld	a,#f2
		;call	Color
		;call	Beep
		ld	a,2			; Has changed
		ret

.notChanged:
		;ld	a,#f6
		;call	Color
		
		ld	a,1			; Has not changed
		ret
DEV_STAT0:

		;ld	a,#fa
		;call	Color
DEV_STAT1:
		xor	a
		ret

Color:
		di
		out	(#99),a
		ld 	a,#87
		out	(#99),a
		;ei
		ret

Beep:
		ld	ix,#c0
		ld	iy,(#fcc1-1)
		call	#1c
		ret

;-----------------------------------------------------------------------------
;
; Obtain logical unit information
;
;Input:   A  = Device index, 1 to 7
;         B  = Logical unit number, 1 to 7
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
;+1 (2): Sector size, 0     IF this information does not apply or is
;        not available.
;+3 (4): Total number of available sectors.
;        0     IF this information does not apply or is not available.
;+7 (1): Flags:
;        bit 0: 1     IF the medium is removable.
;        bit 1: 1     IF the medium is read only. A medium that can dinamically
;               be write protected or write enabled is not considered
;               to be read-only.
;        bit 2: 1     IF the LUN is a floppy disk drive.
;+8 (2): Number of cylinders
;+10 (1): Number of heads
;+11 (1): Number of sectors per track
;
; Number of cylinders, heads and sectors apply to hard disks only.
; For other types of device, these fields must be zero.
;-----------------------------------------------------------------------------
LUN_INFO:
	dec	a
	jp	z,RomDiskLUN_INFO
	
	dec	a
	cp	NUM_SLOTS
	jp	nc,LUN_ERROR
	ld	c,a
	
	ld	a,b
	cp	1
	jp	nz,LUN_ERROR
	
	push	hl
	di
	call	SD_ON
	ld	a,c
	ld	(#5800),a	; SD slot select
	call	GETWRK
	call	GetSectNum	; Get number of available sectors
	;call	WaitReady
	call	nc,WaitBusy
	call	SD_OFF
	ei

	ld	b,h
	ld	c,l		;DEBC = Available sectors
	pop	hl

	jr	c,LUN_ERROR
	
	ld	(hl),0	; +0: Medium type = Block device
	inc	hl
	
	ld	(hl),0	; +1: Sector size = #200
	inc	hl
	ld	(hl),2
	inc	hl
	
	ld	(hl),c	; +3: Total number of sectors
	inc	hl
	ld	(hl),b
	inc	hl
	ld	(hl),e
	inc	hl
	ld	(hl),d
	inc	hl

	ld	(hl),1	; +7: Flags
	inc	hl
	
	ld	(hl),0	; +8 Cylinders
	inc	hl
	ld	(hl),0
	inc	hl
	
	ld	(hl),0	; +10: Heads
	inc	hl
	
	ld	(hl),0	; +11: Sectors per tracks
	
	xor a		; Ok, buffer filled with information.
	ret
	
LUN_ERROR:
	ld	a,1	; Error
	ret
    ENDIF


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
;            .IFORM: Invalid device or logical unit number,
;                    or device not formattable
;        HL = Address of format choice string (in bank 0 or 3),
;             only if A=0 returned.
;             Zero, if only one choice is available.
;
;        When C<>0 at input:
;        A = 0: Ok, device formatted
;            Other: error code, same as DEV_RW plus:
;            .IPARM: Invalid format choice
;            .IFORM: Invalid device or logical unit number,
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
	ld	a,IFORM
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

;-----------------------------------------------------------------------------
; Obtain slot work area (32 bytes) on SLTWRK
; In: A = 0 Slot 1, A= 1 slot 2
; Output: IX = Work area address
;-----------------------------------------------------------------------------
GETWRK:
	push	af
	push	bc
	push	de
	push	hl
	
	call	GetSlot
	ld	b,a
	rrca
	rrca
	rrca
	and	%01100000
	ld	c,a		;C = Slot * 32
	;ld	a,b
	;rlca
	;and	%00011000	;A = Subslot * 8
	;or	c
	;ld	c,a
	ld	b,0
	ld	hl,SLTWRK
	add	hl,bc

	push	hl
	pop	ix

	pop	hl
	pop	de
	pop	bc
	pop	af
	or	a
	ret	z		; Slot 1
	inc	ix		; Slot 2
	ret
	
;---------------------------------------------------------------------------
; Get slot
;---------------------------------------------------------------------------
GetSlot:
		in	a,(#a8);call	RSLREG
		rrca
		rrca
		and	3
		ld	c, a
		ld	b, 0
		ld	hl, EXPTBL
		add	hl, bc
		ld	a, (hl)
		and	80h
		or	c
		ld	c, a
		inc	hl
		inc	hl
		inc	hl
		inc	hl
		ld	a, (hl)
		and	0Ch
		or	c
		ret
;-----------------------------------------------------------------------------
; Set SD control area
;-----------------------------------------------------------------------------
SD_ON:
	push	af
	ld	a,#40
	ld	(#6000),a		;Switch bank to SD control area
	pop	af
	ret

SD_OFF:
	push	af
	ld	a,7 * 2
	ld	(#6000),a		;Restore kernel
	pop	af
	ret
	
;-----------------------------------------------------------------------------
; SD initialize: set to SPI mode
; Out: Cy = Timeout
;      NZ = Error
;
;       Z = Ok
;             E = 0 MMC
;             E = 1 SDSC 1.x
;             E = 2 SDSC 2.0 or higher
;             E = 3 SDHC 2.0 or higher
;-----------------------------------------------------------------------------
InitSD:
	call	InitSD0
	ret	c			; Timeout (card removed or damaged?)
	ret	nz			; Command error

	;call	GETWRK			; Ya deber?a tener en IX el workarea
	
	res	BIT_SDHC,(ix+STATUS)	; Set SDSC as default
	
	ld	a,CARD_SDHC		; Is a SDHC card?
	cp	e
	jr	nz,.notSDHC
	
	set	BIT_SDHC,(ix+STATUS)	; set SDHC flag
	
.notSDHC:
	set	BIT_SD_CHG,(ix+STATUS)	; SD Card has changed
	xor	a
	ret

InitSD0:
	ld	b,10			; Dummy cycle > 76 clocks
InitSD1:
	ld	a,(#5000)		; Quitamos /CS a la tarjeta
	djnz	InitSD1

	call	SD_CMD
	db	#40,0,0,0,0,#95 	; CMD0: Reset

	;call	SD_INIT			; CMD0: Reset & SPI. Hay que hacerlo de forma especial porque sin o falla en el FS-A1 (!?)

	ld	e,#89	; Debug error code
	ret	c	;response timeout
	and	0f3h	;F7=>F3h Changed to support Nokia
	cp	01h
	ld	e,#88	; Debug error code
	ret	nz

InitSD2:
	call	SD_CMD
	db	#48,0,0,#01,#aa,#87 ; CMD8
	ld	e,#87	; Debug error code
	ret	c
	cp	1
	jr	nz,InitSD3	; SD V1.X or MMC

	ex	de,hl
	ld	e,#86	; Debug error code
	ld	a,(hl)
	nop
	ld	a,(hl)
	nop
	ld	a,(hl)
	and	#f
	cp	1
	ret	nz
	ld	a,(hl)
	cp	#aa
	ret	nz	; Wrong voltage range

InitSD2loop:
	call	SD_CMD
	db	#77,0,0,0,0,#95 ; CMD55
	ret	c

	cp	1
	ld	e,#85	; Debug error code
	ret	nz

	call	SD_CMD
	db	#69,#40,0,0,0,#95	;  ACMD41 (HCS = 1)
	
	ld	e,#84	; Debug error code
	ret	c

	and	1
	cp	1
	jr	z,InitSD2loop

	call	SD_CMD
	db	#7a,#00,0,0,0,#95	; CMD58
	
	ld	e,#83	; Debug error code
	ret	c

	ld	e,#82	; Debug error code
	;or      a
	;ret     nz
	ex	de,hl
	ld	a,(hl)	;CSS 32 bits
	cp	(hl)
	cp	(hl)
	cp	(hl)
	bit	6,a	; bit 30

	ld	e,CARD_SD2X
	jr	z,NOT_SDHC
	inc	e
NOT_SDHC:
	xor	a
	ret

InitSD3:
	call	SD_CMD
	db	#77,#00,0,0,0,#95
	ret	c

	bit	2,a
	jr	nz,MMC_FOUND

	cp	1
	ld	e,#92	; Debug error code
	ret	nz

	call	SD_CMD
	db	#69,#00,0,0,0,#95
	ld	e,#93	; Debug error code
	ret	c

	bit	2,a
	jr	nz,MMC_FOUND

	bit	0,a
	jr	nz,InitSD3

	xor	a
	ld	e,CARD_SD1X	; SD card v1.x
	ret

MMC_FOUND:
	call	SD_CMD
	db	#41,#00,0,0,0,#95
	ret	c		;response timeout

	cp	01h
	jr	z,InitSD3

	;call   SetBlockLen
	ld	e,CARD_MMC	; SD 1.x
	or	a	; z=1: OK  z=0: error
	ret

;-----------------------------------------------------------------------------
; Inicializa la SD y pone el modo SPI.
; Si no se hace as? falla en el FS-A1.
; Aparentemente, si se escribe el CRC (#95) desde un registro falla.
;-----------------------------------------------------------------------------
SD_INIT:
	ld	hl,#4000
	ld	a,(hl)		;dummy cycle 8 clocks
	nop			;			[SD_1]
	nop
	ld	(hl),#40	;command
	nop
	ld	(hl),0		;sector(H)
	nop
	ld	(hl),0		;sector(M)
	nop
	ld	(hl),0		;sector(L)
	nop
	ld	(hl),0	;sector(0)
	nop
	ld	(hl),#95	;CRC

	ld	a,(hl)		; CRC
	ld	a,(hl)		; CRC

	ld	bc,#10
.wait:	ld	a,(hl)
	cp	#ff
	ccf
	ret	nc

	djnz	.wait

	scf			;timeout error
	ret
;-----------------------------------------------------------------------------
; In:
;	(DE) = Sector number, 4 bytes
; Out:
;	BCDE  = Sector number
;
; Modify:
;-----------------------------------------------------------------------------
GetSector:
	push	de
	exx
	pop	hl
	
	;ld	a,(ix+CARD_TYPE)
	;cp	CARD_SDHC
	bit	BIT_SDHC,(ix+STATUS)
	jr	z,GetSector2	; Not an SDHC.
	
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	ret
	
GetSector2:
	; Convert sector number to byte offset (sector * 512)
	ld	e,0	; Address 0
	ld	d,(hl)	; Address 1
	inc	hl
	ld	c,(hl)	; Address 2
	inc	hl
	ld	b,(hl)	; Address 3
	
	sla	d
	rl	c
	rl	b	; Address * 2
	ret

;-----------------------------------------------------------------------------
; SD / MMC Access routine
; Input:
;	A	= Command
;	BCDE	= Access address
;
; Output:	Cy = 0 Z=1 A=0 Successful
;		Cy = 1 Z=0 A=Error code
;-----------------------------------------------------------------------------
MMCCMD:
	ld	hl,#4000
	ld	l,(hl)		;dummy cycle 8 clocks
	nop			;			[SD_1]
	nop
	ld	(hl),a		;command
	nop
	ld	(hl),b		;sector(byte 3)
	nop
	ld	(hl),c		;sector(byte 2)
	nop
	ld	(hl),d		;sector(byte 1)
	nop
	ld	(hl),e		;sector(byte 0)
	nop
	ld	(hl),#95	;CRC

	ld	a,(hl)		; CRC
	ld	a,(hl)		; CRC

	ld	l,#00		; BC=#10? B=0
CMD_L1:
	ld	a,(hl)
	cp	#ff
	ccf
	ret	nc
	
	dec	l
	jr	nz,CMD_L1

	scf			;timeout error
	ret
	

;-----------------------------------------------------------------------------
; Sends a command to SD card
; Modify: hl, bc, de, af
; C = Error
;-----------------------------------------------------------------------------
SD_CMD:
	ex	(sp),hl
	ld	de,#4000
	ld	a,(de)		;dummy cycle 8 clocks
	nop			;			[SD_1]
	nop
	ldi			;command
	ldi			;param 31-24
	ldi			;param 23-16
	ldi			;param 15-8
	ldi			;param 7-0
	ldi			;CRC

	ex	(sp),hl
	ld	a,(de)		; CRC
	ld	a,(de)		; CRC

	ld	b,0
SD_CMD2:
	ld	a,(de)
	cp	#ff		; Aqu? se podr?a mirar solo el bit 7? 0=ready
	ccf
	ret	nc

	djnz	SD_CMD2

	scf			;timeout error
	ret


;-----------------------------------------------------------------------------
; Read sectors
;	B    = Number of sectors to read
;	(DE) = First sector number to read
;	HL   = destination address for the transfer
;-----------------------------------------------------------------------------
ReadSD:
	ld	c,2
ReadSD2:
	call	GetSector

	ld	a,#40 + 18	; CMD18: READ_MULTIPLE_BLOCK
	call	MMCCMD
	exx
	jr	c,.timeout	; Timeout

	or	a
	jr	nz,.error2	;Cy=0 A=02

	ex	de,hl		;DE = Destination address
	ld	a,DATA_TOKEN	;start data token
	ld	l,b
	;ld	c,0
.loop:
	;ld	b,2
	ld	h,#40
.wait:
	cp	(hl)		;start data token ?
	jr	nz,.wait

	call	transfer
	
	cp	(hl)		;CRC (dummy)
	cp	(hl)		;CRC (dummy)
	dec	l		;Decrement sectors to read
	jp	nz,.loop

	ld	a,#40 + 12	;CMD12 / stop multiblock read
	call	MMCCMD

	ex	de,hl
	xor	a		;A=00 Cy=0 Successful
	ret
	;************************************************ STOP MULTIPLE BLOCK READ *************
.error:
	ld	a,#40 + 12	;CMD12 / stop multiblock read
	call	MMCCMD
	scf			;Cy=1 
.exit:
	ld	a,NRDY		;error code
	ret

.error2:
.timeout:
	push	bc
	push	de
	push	hl		; Destination address
	call	InitSD
	pop	hl
	pop	de
	pop	bc	
	jr	nz,.error	;response error
	jr	c,.exit		;command error
	
	dec	c
	jp	nz,ReadSD2

	ld	a,DISK		;Other error		[SD_1]
	scf			;Card inserted or removed
	ret


;-----------------------------------------------------------------------------
; Write sectors
;	B    = Number of sectors to write
;	(DE) = First sector number to write
;	HL   = source address for the transfer
; Out:
;	Cy   : 1 = Error
;-----------------------------------------------------------------------------

WriteSD:
	push	hl
	call	GetSector	;BCDE = Sector number
	exx
	pop	hl
.try:
	ld	a,b
	dec	a
	jp	z,Write1	; Only 1 sector

	exx			; BCDE = access address
	ld	a,#40 + 25	; CMD25: WRITE_MULTIPLE_BLOCK
	call	MMCCMD
	exx
	jr	c,.timeout

	or	a
	ld	a,2
	jr	nz,.exit	;command error

	ld	a,(#4000)		;dummy

.loop:

	ld	de,#4000

	ld	a,#FC
	ld	(de),a

	push	bc
	; Transfer data
	call    transfer

	ex	(sp),hl
	ld	a,(de)		;CRC (dummy)
	ex	(sp),hl
	ld	a,(de)		;CRC (dummy)

	pop	bc

	ld	a,(de)		; Dummy

	ld	a,(de)		; Response
	and	#1f
	cp	#5
	;ld	a,3
	jp	nz,.cancel	;response error
	
	;ACMD22 can be used to find the number of well written write blocks
	call	WaitBusy

	;ld	b,e
	djnz	.loop

	ld	a,(de)		; Dummy
	ld	a,(de)		; Dummy

	ld	a,#fd		; Stop transmission
	ld	(de),a
	nop
	nop			; Extra wait for FS-A1
	ld	a,(de)		; Dummy
	ld	a,(de)		; Dummy

	call	WaitBusy

	xor	a		;A=00 Cy=0 Successful operation
	ret


.cancel:
	call	WaitBusy

	ld	a,(de)		; Dummy
	ld	a,(de)		; Dummy

	ld	a,#fd		; Stop transmission
	ld	(de),a
	nop
	nop			; Extra wait for FS-A1
	ld	a,(de)		; Dummy
	ld	a,(de)		; Dummy

	call	WaitBusy
.exit:
	scf			;Cy=1 
	;ld	a,02h		;Cy=1 A=02
	ret

.timeout:
	push	bc		; Number of sector
	push	hl		; Source address
	call	InitSD
	pop	hl
	pop	bc
	
	ld	a,1
	jr	nz,.exit	;response error
	jr	c,.exit		;command error
	jp	.try
	
;
; Write a single sector	
;
Write1:
	exx			; BCDE = access address
	ld	a,#40 + 24	; CMD24: WRITE BLOCK
	call	MMCCMD
	exx
	jp	c,.timeout
	or	a
	jp	nz,.exit	;command error

	ld	de,#4000
	ld	a,(de)		;dummy
	ld	a,DATA_TOKEN	;start data token
	ld	(de),a

	call	transfer
	
	ex	(sp),hl
	ld	a,(de)		;CRC (dummy)
	ex	(sp),hl
	ld	a,(de)		;CRC (dummy)
	
	ld	a,(de)		;dummy
	nop
	ld	a,(de)		;receive data response

	and	#1f
	cp	#05
	jr	nz,.exit	;response error
	;ACMD22 can be used to find the number of well written write blocks

.wait:
	ld	a,(de)
	cp	#ff
	jr	nz,.wait

	xor	a		; Read successfully
	ret

.timeout:
	push	bc		; Number of sector
	push	hl		; Source address
	call	InitSD
	pop	hl
	pop	bc
	
	jr	z,Write1
	jr	nc,Write1

.exit:
	scf			;Cy=1 
	ret

;-----------------------------------------------------------------------------
; Wait until card is not busy
; Modify: AF
;-----------------------------------------------------------------------------	
WaitBusy:
	push	bc

	ld	bc,#8000
.loop:
	ld	a,(#4000)
	or	a
	jr	nz,.end
	
	dec	bc
	ld	a,b
	or	c
	jr	nz,.loop	; Wait while busy

	pop	bc
	scf
	ret
.end:
	pop	bc
	ret
	
	

;-----------------------------------------------------------------------------
; Wait until card is ready
; Modify: AF
;-----------------------------------------------------------------------------
WaitReady:
	push	bc
	ld	bc,#8000
.loop:	
	ld	a,(#4000)
	cp	#ff
	jr	z,.end
	
	dec	bc
	ld	a,b
	or	c
	jr	nz,.loop
	scf
.end:	
	pop	bc
	ret

;-----------------------------------------------------------------------------
; 512 LDIs a bit faster than an LDIR
;-----------------------------------------------------------------------------
transfer:
	REPT 512
	ldi
	ENDM
	ret

	.COMMENT *
;-----------------------------------------------------------------------------
; Set block length
;	Cy   : 1 = Error 0 = Ok
;-----------------------------------------------------------------------------
SetBlockLen:
	ld	b,#40+16	; SET_BLOCKLEN
	ld	c,0
	ld	de,#2
	call	MMCCMD
	ret	c	; Timeout
	
	xor a
	ret
	;*

TestCard:
	call	SD_CMD
	db	#40+16,0,0,2,0,#95
	ret	nc
	
	call	InitSD
	ret	c
	jr	z,TestCard
	scf
	ret

	
;-----------------------------------------------------------------------------
; Read CID register
; Out:
;	Cy   : 1 = Error 0 = Ok
;	HL   : SD data registers
;-----------------------------------------------------------------------------
GetCID:
	call	SD_CMD
	db	#4a,0,0,0,0,#95		; CMD10: SEND_CID
	
	jr	c,GetCID4		; Timeout

	ld	hl,#4000
	ld	a,DATA_TOKEN
	ld	b,0
GetCID2:
	cp	(hl)
	ret	z
	djnz	GetCID2

	scf
	ret

GetCID4:
	call	InitSD			; Modify all registers
	ret	c
	jr	z,GetCID
	scf
	ret
	
;-----------------------------------------------------------------------------
; Get number of available sectors in the card
; Out:
;	DEHL = Number of sectors
;	Cy   : 1 = Error 0 = Ok
;-----------------------------------------------------------------------------
GetSectNum:
	;ld	a,#40+9	; SEND_CSD CMD
	;ld	bc,0
	;ld	de,0
	;call	MMCCMD
	call	SD_CMD
	db	#49,#00,0,0,0,#95	;CMD9: SEND_CSD
	ret	c	; Timeout

	ld	hl,#4000
	ld	a,DATA_TOKEN
	ld	b,0
GetSectNum2:
	cp	(hl)
	jr	z,GetSectNum3
	djnz	GetSectNum2

	scf
	ret

GetSectNum3:
	;ld      de,#c000
	;ld      bc,17
	;ldir		; Read CSD

	bit	BIT_SDHC,(ix+STATUS)
	jr	nz,GetSectNumHC ; SDHC
	
	;ld	a,(ix+CARD_TYPE)
	;cp	CARD_SDHC
	;jr	z,GetSectNumHC ; SDHC
	
	ld	b,5
	call	SkipBytes
	ld	a,(hl)
	and	%1111	; READ_BL_LEN
	ld	c,a

	ld	a,(hl)
	and	%11	; C_SIZE 11-10
	ld	d,a
	ld	e,(hl)
	nop
	ld	a,(hl)
	sla	a
	rl	e
	rl	d
	sla	a
	rl	e
	rl	d
	; DE=C_SIZE

	
	ld	a,(hl)
	and	%11	; C_SIZE_MULT 2-1
	ld	b,(hl)
	sla	b
	rl	a
	;A=C_SIZE_MULT

	add	a,c
	sub	7	; 512 = Sector size
	call	GetExp	; HL = Factor de multiplicacion
	
	ld	b,h
	ld	c,l	; BC = HL
	
	inc	de	; C_SIZE + 1

	call	Mul16	; Numero de sectores = DEHL
	
	ld	b,5
	call	SkipBytes
	
	or	a	; NC = Ok
	ret

GetSectNumHC:
	ld	b,7
	call	 SkipBytes
	ld	a,(hl)
	and	%00111111
	ld	c,a	; C_SIZE 21-16
	
	ld	d,(hl)	; C_SIZE 15-8
	nop
	ld	e,(hl)	; C_SIZE 7-0

	ld	b,6
	call	SkipBytes
	
	inc	de
	ld	h,c
	ld	l,d
	ld	d,e
	ld	e,0

	sla	d
	rl	l
	rl	h

	sla	d
	rl	l
	rl	h

	ex	de,hl
	or	a
	ret

;-----------------------------------------------------------------------------
; Skip B bytes from SD response
;-----------------------------------------------------------------------------
SkipBytes:
	push	af
.loop:
	ld	a,(#4000)
	djnz	.loop
	pop	af
	ret

;-----------------------------------------------------------------------------
; 2^A
;-----------------------------------------------------------------------------
GetExp:
	ld	hl,1
	or	a
	ret	z
GetExp2:
	sla	l
	rl	h
	dec	a
	ret	z
	jr GetExp2

;-----------------------------------------------------------------------------
; Multiplication 16 bits
; DEHL = BC * DE
;-----------------------------------------------------------------------------
Mul16:
	ld hl,0
	ld a,16

Mul16Loop:
	add hl,hl
	rl e
	rl d
	jp nc,NoMul16

	add hl,bc
	jp nc,NoMul16

	inc de
NoMul16:
	dec a
	jp nz,Mul16Loop

	ret

;-----------------------------------------------------------------------------
; Print a zero-terminated string on screen
; Input: DE = String address
;-----------------------------------------------------------------------------
PRINT:
	ld	a,(de)
	or	a
	ret	z
	
	call	CHPUT
	inc	de
	jr	PRINT


;-----------------------------------------------------------------------------
; Includes
;-----------------------------------------------------------------------------
		include	"romdisk.asm"

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
	ret	 			; Yes, quit
	jp	INITXT			; set screen0

.notMSX1:
	ld	c,23h			; Block-2, R#3
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
	xor	a			; Don't displat the function keys
	ld	ix,SDFSCR
	jp	EXTROM

;-----------------------------------------------------------------------------
; Strings
;-----------------------------------------------------------------------------

VERSION_STRING macro v_main, v_sub
db	"version &v_main&.&v_sub&",13,10
endm

TXT_INFO:
		db	"MegaFlashROM SCC+ SD driver "
		VERSION_STRING %DRIVER_VERSION,%DRIVER_SUBVERSION
		db	"(c) 2013 Manuel Pazos",13,10
TXT_EMPTY:		
		db	13,10,0

TXT_INIT:
		db	"SD Card slot ",0

TXT_ROMDSKOK:
		db	"ROM Disk found.",13,10
		db	13,10,0

TXT_MMC:
		db	" MMC",13,10,0
TXT_SD1x:
		db	" SDSC 1.x",13,10,0
TXT_SD2x:
		db	" SDSC 2.x",13,10,0
TXT_SDHC:
		db	" SDHC",13,10,0
	
IDX_TYPE:
		dw	TXT_MMC
		dw	TXT_SD1x
		dw	TXT_SD2x
		dw	TXT_SDHC
	
;-----------------------------------------------------------------------------
; End of the driver code
;-----------------------------------------------------------------------------
DRV_END:

	;ds	3FD0h-(DRV_END-DRV_START)

	end
