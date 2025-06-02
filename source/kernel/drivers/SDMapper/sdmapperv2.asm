	.CPU Z80
	TITLE Nextor driver for SD Mapper v2 v1.1.0
	.relab

; Projeto MSX SD-Mapper v2

; Copyright (c) 2014 Fabio Belavenuto
; Copyright (c) 2017, 2018 Fabio R. Schmidlin 

; This documentation describes Open Hardware and is licensed under the CERN OHL v. 1.1.
; You may redistribute and modify this documentation under the terms of the
; CERN OHL v.1.1. (http://ohwr.org/cernohl). This documentation is distributed
; WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY,
; SATISFACTORY QUALITY AND FITNESS FOR A PARTICULAR PURPOSE.
; Please see the CERN OHL v.1.1 for applicable conditions

; Technical info:
; The SD-Mapper v2 uses an ASCII16 ROM mapper and the following register set is only enabled at page-7.
;
; 7B00h~7EFFh	: SPI data transfer window (read/write)
; 7FF0h		: Interface status and card select register (read/write)
;	<read>
;	If no SD card is selected:
;	    b7-b2 : always 0
;	    b1 : SW1 status (Driver selection)
;	    b0 : SW0 status. 0=RAM disabled, 1=RAM enabled
;	If any SD card is selected:
;	    b7-b3 : always 0
;	    b2 : 1=Write protecton enabled for SD card-slot selected
;	    b1 : 0=SD card present in the selected card-slot
;	    b0 : 1=SD Card on slot selected changed since last read
;	<write>
;	    b0 : SD card slot-0 chip-select (1=selected)
;	    b1 : SD card slot-1 chip-select (1=selected)

; 7FF1h		: 8-bit timer (97.65625 KHz frequency, 10.24uS resolution) (read/write)
; When a value is written, the timer will decrease it until it reaches zero
;
;
; A Special thanks goes to Elm-chan, for publishing the documentation we used to
; create this driver. http://elm-chan.org/docs/mmc/mmc_e.html

;-----------------------------------------------------------------------------
;
; Driver configuration constants
;

CMDTIMEOUT	equ	2	; Command accepted timeout	: 2*2.6nS
READYTIMEOUT	equ	385	; Card ready timeout		: 10 second
DETTIMEOUT	equ	385	; Detection timeout		: 385*2.6nS = 1 second

; DEFINES
TURBOINIT 	equ 1	; Execute DRV_INIT with the turbo enabled 
DSKEMU 		equ 1	; Include a built-in floppy disk emulator
; PARALLELCARD	equ 1 	; Allows parallel card processing. Seems to require
			; independent communication lines for each card
; HASMEGARAM	equ 1	; Driver for an SD-Mapper with MegaRAM
; DEBUG1	equ 1	; Enable level-1 debugging (Only status queries)
; DEBUG2	equ 1	; Enable level-2 debugging (RW calls)
; DEBUG3	equ 1	; Enable level-3 debugging (requires level-1). Also logs invalid DEVs and LUN numbers


;Driver type:
;   0 for drive-based
;   1 for device-based

DRV_TYPE	equ	1

;Hot-plug devices support (device-based drivers only):
;   0 for no hot-plug support
;   1 for hot-plug support

DRV_HOTPLUG	equ	1


;Driver version

VER_MAIN	equ	1
VER_SEC		equ	1
VER_REV		equ	0

;-----------------------------------------------------------------------------
; SPI addresses. Check the Technical info above for the bit contents

SPIDATA		equ #7B00		; read/write
SPICTRL		equ #7FF0		; write
SPISTATUS	equ #7FF0		; read
TIMERREG	equ #7FF1		; read/write

; Interface status flags
IF_RAM		equ 0		; 1=Interface RAM is enabled
IF_DRVER	equ 1		; RAM mode: 0=MegaRAM, 1=MemoryMapper
IF_M_RAM	equ (1 shl IF_RAM)		; bitmask for IF_RAM
IF_M_DRVER	equ (1 shl IF_DRVER)		; bitmask for IF_DRVER

; card slot status flags
SD_DSKCHG	equ 0		; SD card changed since last status check
SD_PRESENT	equ 1		; SD card present
SD_WRTPROT	equ 2		; SD card is write protected
SD_M_DSKCHG	equ (1 shl SD_DSKCHG)		; bitmask for SD_DSKCHG
SD_M_PRESENT	equ (1 shl SD_PRESENT)		; bitmask for SD_PRESENT
SD_M_WRTPROT	equ (1 shl SD_WRTPROT)		; bitmask for SD_WRTPROT

; SPI commands: 
CMD0	equ 0  OR #40
CMD1	equ 1  OR #40
CMD8	equ 8  OR #40
CMD9	equ 9  OR #40
CMD10	equ 10 OR #40
CMD12	equ 12 OR #40
CMD13	equ 13 OR #40
CMD16	equ 16 OR #40
CMD17	equ 17 OR #40
CMD18	equ 18 OR #40
CMD24	equ 24 OR #40
CMD25	equ 25 OR #40
CMD55	equ 55 OR #40
CMD58	equ 58 OR #40
ACMD23	equ 23 OR #40
ACMD41	equ 41 OR #40

; SD card errors
R1ERR.IDLE	equ 1	; In idle state
R1ERR.ERARST	equ 2	; Erase reset
R1ERR.ILLGCMD	equ 3	; Illegal command
R1ERR.CRCERR	equ 4	; Communication CRC error
R1ERR.ERAERR	equ 5	; Erase sequence error
R1ERR.ADDRERR	equ 6	; Address error
R1ERR.PARMERR	equ 7	; Parameter error
R1ERR.M_IDLE	equ (1 shl R1ERR.IDLE)
R1ERR.M_ERARST	equ (1 shl R1ERR.ERARST)
R1ERR.M_ILLGCMD	equ (1 shl R1ERR.ILLGCMD)
R1ERR.M_CRCERR	equ (1 shl R1ERR.CRCERR)
R1ERR.M_ERAERR	equ (1 shl R1ERR.ERAERR)
R1ERR.M_ADDRERR	equ (1 shl R1ERR.ADDRERR)
R1ERR.M_PARMERR	equ (1 shl R1ERR.PARMERR)

WRKAREA.TRLDIR		equ 0			; Pointer to the R800 data transfer helper
WRKAREA.NUMSD		equ WRKAREA.TRLDIR+2		; Currently selected card: 1 or 2 
WRKAREA.CARDFLAGS	equ WRKAREA.NUMSD+1		; Flags that indicate card-change or card error 
					; b0: card1 error
					; b1: card2 error
					; b2: card1 LUN changed flag
					; b3: card2 LUN changed flag
					; b4: card1 DEV changed flag
					; b5: card2 DEV changed flag
					; b6: card1 version
					; b7: card2 version
WRKAREA.NUMBLOCKS	equ WRKAREA.CARDFLAGS+1		; Number of blocks in multi-block operations 
WRKAREA.TEMP		equ WRKAREA.NUMBLOCKS+1		; Buffer for temporary data 
WRKAREA.DSKEMU.DSKDEV	equ WRKAREA.TEMP+1		; Floppy emulation: b0~b2=DEV, b3~b7=Disk number 
;WRKAREA.DSKEMU.VDPDR	equ WRKAREA.DSKEMU.DSKDEV+1	; VDP.DR cached value
WRKAREA.DSKEMU.VDPDW	equ WRKAREA.DSKEMU.DSKDEV+1	; VDP.DW cached value
WRKAREA.END		equ WRKAREA.DSKEMU.VDPDW+1

; WRKAREA.CARDFLAGS shift amount and direction
WCF_N_ERROR	equ 0	; SD card error
WCF_L_LUNCHG	equ 2	; LUN has changed software flag
WCF_L_DEVCHG	equ 4	; Device has changed software flag
WCF_R_CRDVER	equ 2	; Current card version

CID.MID		equ 0			; Manufacturer ID
CID.OID		equ CID.MID+1		; OEM ID
CID.PNM		equ CID.OID+2		; Product Name
CID.PRV		equ CID.PNM+5
CID.PSN		equ CID.PRV+1		; Product Serial Number
CID.RSVMDT	equ CID.PSN+4		; Reserved, Manufacturing date
CID.CRC		equ CID.RSVMDT+2	; b7=1, b6~b0 = CRC7 checksum
CID.END		equ CID.CRC+1

;-----------------------------------------------------------------------------
;
; Standard BIOS and work area entries
CALSLT	equ #001C		; Call routine in any slot
CALLF	equ #0030		; Call routine in any slot
INITXT	equ #006C		; Inicializa SCREEN0
CHSNS	equ #009C		; Sense keyboard buffer for character
CHGET	equ #009F		; Get character from keyboard buffer
CHPUT	equ #00A2		; A=char
BEEP	equ #00C0		; Does a beep
CLS	equ #00C3		; Chamar com A=0
ERAFNK	equ #00CC		; Erase function key display
SNSMAT	equ #0141		; Read row of keyboard matrix
KILBUF	equ #0156		; Clear keyboard buffer
EXTROM	equ #015F
CHGCPU	equ #0180		; Change the turbo mode
GETCPU	equ #0183		; Get the turbo mode

; subROM functions
SDFSCR	equ #0185
REDCLK	equ #01F5


; System variables
VDP.DR	equ #0006
VDP.DW	equ #0007
MSXVER	equ #002D
LINL40	equ #F3AE		; Width
LINLEN	equ #F3B0
TEMP3	equ #F69D
INTFLG	equ #FC9B
SCRMOD	equ #FCAF
EXPTBL	equ #FCC1
RG15SA	equ #FFEE		; VDP Register 15 Save copy.



;-----------------------------------------------------------------------------


	org		#4000

	ds		256, #FF		; 256 dummy bytes

DRV_START:

;-----------------------------------------------------------------------------
;
; Miscellaneous constants
;

;This is a 2 byte buffer to store the address of code to be executed.
;It is used by some of the kernel page 0 routines.

CODE_ADD:	equ	#F1D0


;-----------------------------------------------------------------------------
;
; Error codes for DEV_RW
;

ENCOMP	equ	0FFh
EWRERR	equ	0FEh
EDISK	equ	0FDh
ENRDY	equ	0FCh
EDATA	equ	0FAh
ERNF	equ	0F9h
EWPROT	equ	0F8h
EUFORM	equ	0F7h
ESEEK	equ	0F3h
EIFORM	equ	0F0h
EIDEVL	equ	0B5h
EIPARM	equ	08Bh

;-----------------------------------------------------------------------------
;
; Macros
;

BYTE2STR: MACRO value
 IF (value GT 99)
	db	((value / 100) MOD 10) + #30
 ENDIF
 IF (value GT 9)
	db	((value / 10) MOD 10) + #30
 ENDIF
	db	(value MOD 10) + #30
ENDM

IFDEF DEBUG1
 PRTSTR: MACRO string
	call	PRTSTRCALL
	DEFZ string
 ENDM
ENDIF

;-----------------------------------------------------------------------------
;
; Routines and information available on kernel page 0
;

;* Get in A the current slot for page 1. Corrupts F.
;  Must be called by using CALBNK to bank 0:
;    xor a
;    ld ix,GSLOT1
;    call CALBNK

GSLOT1	equ	402Dh


;* This routine reads a byte from another bank.
;  Must be called by using CALBNK to the desired bank,
;  passing the address to be read in HL:
;    ld a,<bank number>
;    ld hl,<byte address>
;    ld ix,RDBANK
;    call CALBNK

RDBANK	equ	403Ch


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

CALLB0	equ	403Fh


;* Call a routine in another bank.
;  Must be used if the driver spawns across more than one bank.
;
;  Input:  A = bank number
;          IX = routine address
;          AF' = AF for the routine
;          HL' = Ix for the routine
;          BC, DE, HL, IY = input for the routine
;  Output: AF, BC, DE, HL, IX, IY returned from the called routine.

CALBNK	equ	4042h


;* Get in IX the address of the SLTWRK entry for the slot passed in A,
;  which will in turn contain a pointer to the allocated page 3
;  work area for that slot (0 if no work area was allocated).
;  If A=0, then it uses the slot currently switched in page 1.
;  Returns A=current slot for page 1, if A=0 was passed.
;  Corrupts F.
;  Must be called by using CALBNK to bank 0:
;    ld a,<slot number> (xor a for current page 1 slot)
;    ex af,af'
;    xor a
;    ld ix,GWORK
;    call CALBNK

GWORK	equ	4045h


;* This address contains one byte that tells how many banks
;  form the Nextor kernel (or alternatively, the first bank
;  number of the driver).

K_SIZE	equ	40FEh


;* This address contains one byte with the current bank number.

CUR_BANK	equ	40FFh


;-----------------------------------------------------------------------------
;
; Built-in format choice strings
;

NULL_MSG  equ     781Fh	;Null string (disk can't be formatted)
SING_DBL  equ     7820h ;"1-Single side / 2-Double side"


;-----------------------------------------------------------------------------
;
; Driver signature
;
	db	"NEXTOR_DRIVER",0


;-----------------------------------------------------------------------------
;
; Driver flags:
;    bit 0: 0 for drive-based, 1 for device-based
;    bit 1: 1 for hot-plug devices supported (device-based drivers only)
;    bit 2: 1 if the driver implements the DRV_CONFIG routine

	db DRV_TYPE + 2 * DRV_HOTPLUG + 4

;-----------------------------------------------------------------------------
;
; Reserved byte
;
	db	0

;-----------------------------------------------------------------------------
;
; Driver name
;
; It will be shown in the FDISK interface selection menu

DRV_NAME:
	db	"FBLabs SDXC"
	ds	32-($-DRV_NAME)," "


;-----------------------------------------------------------------------------
;
; Jump table for the driver public routines
;

	; These routines are mandatory for all drivers
        ; (but probably you need to implement only DRV_INIT)

	jp	DRV_TIMI
	jp	DRV_VERSION
	jp	DRV_INIT
	jp	DRV_BASSTAT
	jp	DRV_BASDEV
	jp	DRV_EXTBIO
	jp	DRV_DIRECT0
	jp	DRV_DIRECT1
	jp	DRV_DIRECT2
	jp	DRV_DIRECT3
	jp	DRV_DIRECT4
	jp	DRV_CONFIG

	ds	12

	; These routines are mandatory for device-based drivers

	jp	DEV_RW
	jp	DEV_INFO
	jp	DEV_STATUS
	jp	LUN_INFO


;=====
;=====  END of data that must be at fixed addresses
;=====


;-----------------------------------------------------------------------------
;
; Timer interrupt routine, it will be called on each timer interrupt
; (at 50 or 60Hz), but only if DRV_INIT returns Cy=1 on its first execution.

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
;      Cy = 1 if DRV_TIMI must be hooked to the timer interrupt, 0 otherwise
;
; 2) Second execution, for work area and hardware initialization.
;    Input:
;      A = 1
;      B = number of allocated drives for this controller
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
	or	a		; Is this the 1st call? 
	jr	nz,.call2	; No, skip
; 1st call:

	; ***Workaround for Nextor not passing the status of the CTRL
	; and SHIFT keys at the moment of the beep to the DRV_CONFIG
	; and DRV_INIT functions 
	call	getWorkArea		; IX=Work area pointer
	ld	a,6
	call	SNSMAT
	ld	(IX+WRKAREA.TEMP),a	; TEMP = KBD-row6

	ld	hl,0		; No extra space required
	ld	a,(MSXVER)
 IFDEF DSKEMU
	or	a
	ret	z
;	ld	a,(VDP.DR)
;	ld	(IX+WRKAREA.DSKEMU.VDPDR),a
	ld	a,(VDP.DW)
	ld	(IX+WRKAREA.DSKEMU.VDPDW),a
	ld	a,(MSXVER)
 ENDIF
	cp	3		; MSX Turbo-R?
	ccf
	ret	nc		; No, return with Cy off
	ld	hl,R800DATHLP.end-R800DATHLP	; size of the extra work area needed for the TR
	or	a		; Clear Cy
	ret


.call2:
; 2nd call: 
 IFDEF TURBOINIT
	ld	a,(CHGCPU)
	cp	#C3		; IS CHGCPU present?
	jr	nz,.call2ini
	call	GETCPU
	push	af		; Save the current CPU
	ld	a,#82
	call	CHGCPU		; Enable the turbo
.call2ini:
 ENDIF ; TURBOINIT
	call	MYSETSCR		; Set the screen mode
	call	getWorkArea		; IX=Work area pointer

;	; Clear the rest of my SLTWRK area 
;	push	ix
;	pop	hl
;	inc	hl			; Skip the pointer to the additional work Area
;	inc	hl
;	xor	a
;	ld	b,6
;.loopclr:
;	ld	(hl),a
;	inc	hl
;	djnz	.loopclr
	
	ld	de,strTitle		; prints the title 
	call	printString

.sdhcinit:	; FBLabs SDXC Interface initialization
	call	.printmode		; Print the switches configuration
	ld	a,#3C			; Initialize the card flags
	ld	(ix+WRKAREA.CARDFLAGS), a

	ld	a, 1			; detectar cartao 1
	call	.detecta
	ld	a, 2			; detectar cartao 2
	call	.detecta

	call	INSTR800HLP		; Install R800 data copy on workarea

 IFDEF DSKEMU
	call	DETIMG	; Detect a floppy disk image
	ld	a,(IX+WRKAREA.DSKEMU.DSKDEV)	
	or	a			; Disk emulation enabled? 
	ld	de,strDskEmu
	call	nz,printString		; Yes, print message
 ENDIF

	call	INICHKSTOP		; Check if the STOP key was pressed

	ld	de, strCrLf
 
	call	printString
.drv_init_end:
	; ***Workaround for a bug in Nextor that causes it to freeze if
	; CTRL+STOP was pressed on boot
	call	CLRCTRLSTOP

.restCPU:	; Restore the CPU if necessary
 IFDEF TURBOINIT
	ld	a,(CHGCPU)
	cp	#C3		; IS CHGCPU present?
	ret	nz
	pop	af
	or	#80
	jp	CHGCPU
 ELSE
	ret
 ENDIF ; TURBOINIT


;------- DRV_INIT aux routines ----------

.detecta:
	ld	(ix+WRKAREA.NUMSD), a	; Save the requested card slot
	ld	de, strCartao
	call	printString
	ld	c, (ix+WRKAREA.NUMSD)
	ld	a,c
	add	'0'
	call	CHPUT
	ld	a, ':'
	call	CHPUT
	ld	a, ' '
	call	CHPUT
	ld	a,c			; Get card slot#
;	cpl				; invert bits
;	and	3
	ld	(SPICTRL), a		; Select card slot
	ld	a, (SPISTATUS)		; get card slot status
	call	disableSDs
	and	SD_M_PRESENT		; Is there an card present?
	jr	z,.naoVazio		; Yes, skip
	ld	de, strVazio		; Empty SD card slot message
	jp	printString
;	jp	.marcaErro
.naoVazio:
	call	detectaCartao		; tem cartao no slot, inicializar e detectar
	jr	nc,.detectou
	ld	de, strNaoDetectado
	jp	printString
;.marcaErro:
;	jp	marcaErroCartao		; slot vazio ou erro de deteccao, marcar nas flags
.detectou:
	call	getCardVer
	ld	de, strSDV1	; e imprimir
;	or	a
	jr	z,.pula1
	ld	de, strSDV2
.pula1:
	call	printString

	ld	de,CMD10*256+CID.MID	; Point to the Manufacturer ID
	call	setCxDrd
	ld	hl,SPIDATA
	ld	c,(hl)		; c=Manufacturer ID

	ld	b,18-1		; 16 data bytes and 2 response bytes -1 
	call	flushCxD	; Flush the rest of the CID data

	ld	a,c		; pegar byte do fabricante

	ld	hl,TEMP3
	call	HexToAscii
	ld	a,(TEMP3)
	call	CHPUT
	ld	a,(TEMP3+1)
	call	CHPUT
	ld	a,'h'
	call	CHPUT
	ld	a,')'
	call	CHPUT
	ld	a,' '
	call	CHPUT
	ld	a,c		; pegar byte do fabricante
	call	getMakerName	; de = Maker string 
	call	printString	; e imprimir
	ld	de,strCrLf
	call	printString
	jp	disableSDs


.printmode:		; Print the two switches configuration
	xor	a			; 0=Interface status
	ld	(SPICTRL),a
	ld	a, (SPISTATUS)		; Check if the mapper/megaRAM is active
	and	IF_M_RAM		; Is the RAM enabled?
	ld	de,strMr_mp_desativada
 IFDEF HASMEGARAM
	jr	z,.print		; No, skip
	ld	a, (SPISTATUS)		; ativa, testar se eh mapper ou megaram
	and	IF_M_DRVER
	ld	de,strMapper
	jr	nz,.print
	ld	de, strMegaram		; Megaram ativa
.print:
	jp	printString
 ELSE
	call	z,printString		; Yes, print
	ld	a, (SPISTATUS)		; Get the MainBIOS/DevBIOS switch status
	and	IF_M_DRVER
	ld	de,strDrvMain
	jr	nz,.printdrv
	ld	de, strDrvDev
.printdrv:
	jp	printString
 ENDIF





;-----------------------------------------------------------------------------
;
; Obtain driver version
;
; Input:  -
; Output: A = Main version number
;         B = Secondary version number
;         C = Revision number

DRV_VERSION:
	ld	a, VER_MAIN
	ld	b, VER_SEC
	ld	c, VER_REV
	ret


;-----------------------------------------------------------------------------
;
; BASIC expanded statement ("CALL") handler.
; Works the expected way, except that if invoking CALBAS is needed,
; it must be done via the CALLB0 routine in kernel page 0.

DRV_BASSTAT:
	scf
	ret


;-----------------------------------------------------------------------------
;
; BASIC expanded device handler.
; Works the expected way, except that if invoking CALBAS is needed,
; it must be done via the CALLB0 routine in kernel page 0.

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
; Calls to addresses 7850h, 7853h, 7856h, 7859h and 785Ch
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
; Get driver configuration 
; (bit 2 of driver flags must be set if this routine is implemented)
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
;     C = Relative drive number at boot time (0~n)
;   Output:
;     B = Device index (1~7)
;     C = LUN index
;
;
; Note: The boot sequence is a bit unconventional. First, the system boots
; in DOS2 mode, and calls DRV_CONFIG with A=1, if CTRL wasn't pressed. If
; CTRL was pressed, it assumes 1 drive per interface without that call to
; DRV_CONFIG.
; After all devices are configured, the system calls DRV_CONFIG with A=2
; just before it loads the NEXTOR.SYS file
; At this point, it checks if the boot sector is DOS1, and also checks
; if the "1" key is pressed. I either case, it will now boot the DOS1
; kernel from the ROM, so this kernel calls again DRV_CONFIG with A=1 and
; a bit later with A=2.


; Note-2: I opened two bug reports related to the DRV_CONFIG function
; https://github.com/Konamiman/Nextor/issues/12
; https://github.com/Konamiman/Nextor/issues/15


DRV_CONFIG:
 IFDEF DEBUG1
	push	af
	ld	a,'C'		; DRV_CONFIG debug ID
	call	PRTCHAR
	pop	af
	push	af
	add	'0'		; Config parameter 
	call	PRTCHAR
	ld	a,b		; DOS mode 
	add	'0'
	call	PRTCHAR
	pop	af
 ENDIF
	cp	1
	jr	nz,.tryC2

	; Config-1: Get number of drives at boot time
	; ***This function is called with the BIOS still present
	; on the frame-0

	djnz	.twoDrives	; Always request 2 drives for DOS2 mode
				; it will be ignored if CTRL is pressed

.C1dos1:	; DOS1 mode is only executed way after DOS2 mode
	; ***This was a workaround for the problem that Nextor
	; 2.0.5-beta1 crashes on DOS1 mode if CTRL is pressed and the
	; interface still requests 2 drives.

	call	getWorkArea		; IX=Work area pointer
	bit	1,(IX+WRKAREA.TEMP)	; ***Was CTRL pressed on boot?
	jr	nz,.twoDrives		; No, request 2 drives

;.oneDrive:
	xor	a
	ld	b,1
	ret

.twoDrives:
	xor	a
	ld	b,2
	ret



;----------------
.tryC2: cp	2
	jr	nz,.notavail
	; Config-2: Get default configuration for drive
	; ***This function is called without the BIOS presence
	; on the frame-0
 IFDEF DEBUG1
	push	af
	ld	a,c
	add	'0'
	call	PRTCHAR
	pop	af
 ENDIF
	; Workaround for the Nextor CTRL+STOP freeze bug on boot
	call	CLRCTRLSTOP

	call	getWorkArea	; IX=Work area pointer

	ld	a,(IX+WRKAREA.DSKEMU.DSKDEV)
	or	a		; Is the floppy disk emulation enabled? 
	jr	z,.c2Normal	; No, skip and use the normal drive mapping 

	and	7
	cp	1		; Is it the DEV1
	jr	z,.c2Normal	; Yes, use a normal drive mapping

	; Swap the DEV/LUNs
	ld	a,2
	sub	c
	ld	b,a
	ld	c,1
	xor	a
	ret


.c2Normal:	; Normal drive config
 IFDEF DEBUG1
	ld	a,'n'
	call	PRTCHAR
 ENDIF
	ld	b,c
	inc	b	; DRV x = DEV x+1 
	ld	c,1	; LUN=1
	xor	a
	ret

;----------------
.notavail:
	ld	a,1
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

DEV_RW:
	push	af
	cp	3		; somente 2 dispositivos
	jr	nc,.errorIDEVL
	dec	c		; somente 1 logical unit
	jr	nz,.errorIDEVL

	call	getWorkArea	; IX=Work area pointer
	ld	(ix+WRKAREA.NUMBLOCKS),b	; save the number of blocks to transfer 
	push	de
	push	hl
	call	slctNchkCard	; Select and check the card
	ld	c,e		; c=error code
	pop	hl
	pop	de
	jr	c,.errorCard
	jr	nz,.cardOk

.errorNRDY:
	ld	c,ENRDY		; Not ready
	call	disableSDs
 IFDEF DEBUG1
	jr	.errorCard2
 ELSE
	jr	.popQuitIniError
 ENDIF

.errorCard:
	ld	c,ENCOMP	; Incompatible disk
	call	disableSDs
.errorCard2:
 IFDEF DEBUG1
	pop	af
	ld	a,'R'
	jr	nc,.errorCard3
	ld	a,'W'
.errorCard3:
	call	PRTCHAR
	PRTSTR	"11n"
	jr	.quitIniError
 ELSE
	jr	.popQuitIniError
 ENDIF

.errorIDEVL:
 IFDEF DEBUG3
	PRTSTR	"RW"
	add	'0'
	call	PRTCHAR
;	ld	a,c		; Not implemented
;	add	'0'
;	call	PRTCHAR
	ld	a,'i'
	call	PRTCHAR
 ENDIF
	ld	c,EIDEVL	; error: Invalid device or LUN 
.popQuitIniError:		; Pop and quit on initialization error
	pop	af		; flush the stack
.quitIniError:
	ld	a,c		; A=error code
	ld	b,0		; 0 sectors r/w
	ret

.cardOk:
	call	SETLDIRHLPR	; hl'=Pointer to LDIR helper in RAM
	pop	af		; a=Device number, f=read/write flag 
	jr	c,DEV_W		; Skip if it's a write operation 

DEV_R:
 IFDEF DEBUG1
	ld	a,'R'
	call	PRTCHAR
 ENDIF
	push	de
	pop	iy
 IFDEF DSKEMU
	call	GETSECT	; Get the sector number
	ld	a,EIDEVL	; error: Invalid device or LUN 
	ret	c
 ELSE
	ld	e,(iy+0)	; BC:DE=sector number
	ld	d,(iy+1)
	ld	c,(iy+2)
	ld	b,(iy+3)
 ENDIF

	call	LerBloco	; Low-level sector read
 IFDEF DEBUG1
	jp	nc,PRTSEMIC
	ld	a,'e'
	call	PRTCHAR
 ELSE
	ret	nc		; Return with A=0 if no error occurred
 ENDIF
	call	marcaErroCartao		; ocorreu erro, marcar nas flags
	ld	a,(ix+WRKAREA.NUMBLOCKS) ; Get the number of requested blocks
	sub	iyh		; subtract the number of remaining blocks
	ld	b,a		; b=number of blocks written
	ld	a,e		; Get the error code
	ret

DEV_W:
 IFDEF DEBUG1
	ld	a,'W'
	call	PRTCHAR
 ENDIF
	; Test if the card is write protected
	ld	a,(SPISTATUS)	; Get this card slot status
	and	SD_M_WRTPROT	; Is the card write protected?
	jr	z,.ok
 IFDEF DEBUG1
	PRTSTR	"!p"
 ENDIF
	ld	a, EWPROT	; disco protegido
	ld	b,0		; 0 blocks were written
	jp	disableSDs
.ok:
 IFDEF DEBUG1
	PRTSTR	"11"
 ENDIF
	push	de
	pop	iy

 IFDEF DSKEMU
	call	GETSECT	; Get the sector number
	ld	a,EIDEVL	; error: Invalid device or LUN 
	ret	c
 ELSE
	ld	e,(iy+0)	; BC:DE=sector number
	ld	d,(iy+1)
	ld	c,(iy+2)
	ld	b,(iy+3)
 ENDIF

	call	GravarBloco	; Low-level sector save
 IFDEF DEBUG1
	jp	nc,PRTSEMIC
	ld	a,'e'
	call	PRTCHAR
 ELSE
	ret	nc		; Return with A=0 if no error occurred
 ENDIF
	call	marcaErroCartao		; ocorreu erro, marcar nas flags
	ld	a,(ix+WRKAREA.NUMBLOCKS) ; Get the number of requested blocks
	sub	iyh		; subtract the number of remaining blocks
	ld	b,a		; b=number of blocks written
	ld	a,e		; Get the error code
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
;+0 (1): Numer of logical units, from 1 to 7. 1 if the device has no logical
;        units (which is functionally equivalent to having only one).
;+1 (1): Device flags, always zero in Beta 2.
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
	or	a		; dev=0 isn't allowed
	jr	z,.nodev
	cp	3		; Max 2 devices 
	jr	c,.devOk
.nodev:
	ld	a,1		; invalid device index
	ret
.devOk:
 IFDEF DEBUG1
	push	af
	ld	a,'D'		; DEV_INFO debug ID
	call	PRTCHAR
	pop	af
	push	af
	add	'0'		; Device number
	call	PRTCHAR
	ld	a,b		; Info number
	add	'0'
	call	PRTCHAR
	pop	af
 ENDIF
	call	getWorkArea	; IX=Work area pointer
	inc	b
	djnz	.notBasicInfo

; Basic information:
	push	hl
	call	slctNchkCard	; Select and check the card
	pop	hl
	call	disableSDs
 IFDEF DEBUG1
	jr	nz,.hasCard
	ld	a,'n'		; DEV_INFO return status: no card
	call	PRTCHAR
	ld	a,2
	ret
 ELSE
	ld	a,2
	ret	z		; No card: quit with "dev not available"
 ENDIF
 IFDEF DEBUG1
.hasCard:
	ld	a,'0'		; DEV_INFO return status: no card
	call	PRTCHAR
 ENDIF
	ld	(hl),1		; 1 logical unit somente
	inc	hl
	xor	a
	ld	(hl),a		; reservado, deve ser 0
	ret			; retorna com A=0 (OK)

.notBasicInfo:
	push	hl
	push	bc
	call	slctNchkCard	; Select and check the card
	pop	bc
	pop	hl
	jr	c,.cardError	; Card error? No info
	jr	nz,.notBasicInfo2 ; Card present, get its info
 IFDEF DEBUG1
	ld	a,'n'		; DEV_INFO return status: no card
	call	PRTCHAR
 ELSE
.cardError:
.noInfo:
 ENDIF
	ld	a,2		; Quit with "Info not available" status
	jp	disableSDs
 IFDEF DEBUG1
.cardError:
	ld	a,'e'		; DEV_INFO return status: error 
	call	PRTCHAR
	ld	a,2		; Quit with "Info not available" status
	ret
.noInfo:
	ld	a,'u'		; DEV_INFO return status: Uknown info 
	call	PRTCHAR
	ld	a,2		; Quit with "Info not available" status
	jp	disableSDs
 ENDIF

.notBasicInfo2:
	djnz	.tryDevInfo
; Manufacturer Name:
	ld	de,CMD10*256+CID.MID	; Point to the Manufacturer ID
	call	setCxDrd
	jr	c,.noInfo
	ex	de,hl		; de=Nextor buffer pointer
	ld	hl,SPIDATA
	ld	c,(hl)		; c=Manufacturer ID
	ld	b,18-1-CID.MID
	call	flushCxD	; Flush the rest of the CID data
	call	disableSDs

	ld	h, d		; Save the Nextor buffer pointer 
	ld	l, e
	ld	b,64		; Fill the buffer with spaces 
	ld	a,' '
.loop1:
	ld	(hl),a
	inc	hl
	djnz	.loop1
	ex	de,hl		; hl=pointer to Nextor buffer 

	ld	(hl),'('	; Place "(0xNN) " on the buffer
	inc	hl
	ld	(hl),'0'
	inc	hl
	ld	(hl),'x'
	inc	hl
	ld	a,c		; Get the Manufacturer ID
	push	bc
	call	HexToAscii
	pop	bc
	ld	(hl),')'
	inc	hl
	ld	(hl),' '
	inc	hl
	ld	a,c		; Get the Manufacturer ID
	call	getMakerName	; de = Pointer to the Maker string 

	; Copy the Maker name to the buffer
.loop2:
	ld	a,(de)
	ld	(hl),a
	inc	hl
	inc	de
	add	a,a
	jr	nc,.loop2
	dec	hl
	res	7,(hl)		; Clear the bit7 of the last char
 IFDEF DEBUG1
	PRTSTR	"=0"
 ENDIF
	xor	a		; Return with A=0 (Ok)
	ret

.tryDevInfo:
	djnz	.trySerial
; Device Name:
	ld	de,CMD10*256+CID.PNM	; Point to the Product Name 
	call	setCxDrd
	jr	c,.noInfo
	push	hl
	ex	de,hl		; de=Nextor buffer pointer
	ld	hl,SPIDATA
	ld	bc,5		; 5 chars 
	ldir			; copy the product name

	ld	b,18-5-CID.PNM
	call	flushCxD	; Flush the rest of the CID data
	call	disableSDs

	pop	hl		; Restore the original Nextor pointer
	ld	b,5		; b=string size
	call	STR_SANITIZE	; Sanitize the string before sending it to Nextor

	ex	de,hl		; hl=current pointer to the Nextor Buffer 
	ld	a,64
	sub	b		; Fills the rest with spaces
	ld	b,a
	jr	.fillspaces

.trySerial:
	dec	b
	jp	nz,.noInfo
; Serial Number:
	ld	(hl),'0'	; Coloca prefixo "0x"
	inc	hl
	ld	(hl),'x'
	inc	hl

	ld	de,CMD10*256+CID.PSN	; Point to the Product Serial # 
	call	setCxDrd
	jp	c,.noInfo

	ld	b,4		; 4 bytes do serial
.loop3:
	ld	a,(SPIDATA)
	call	HexToAscii	; converter HEXA para ASCII
	djnz	.loop3

	ld	b,18-4-CID.PSN
	call	flushCxD	; Flush the rest of the CID data
	call	disableSDs

	ex	de,hl		; hl=Current Nextor buffer position
	ld	b,54		; Fill the rest with spaces 
.fillspaces:
	ld	a,' '
.loop4:
	ld	(hl), a
	inc	hl
	djnz	.loop4
 IFDEF DEBUG1
	PRTSTR	"=0"
 ENDIF
	xor	a		; Return with A=0 (Ok)
	ret

;-----------------------------------------------------------------------------
;
; Obtain device status
;
;Input:   A = Device index, 1 to 7
;         B = Logical unit number, 1 to 7
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
;
; The returned status is always relative to the previous invokation of
; DEV_STATUS itself. Please read the Driver Developer Guide for more info.

DEV_STATUS:
	ld	d,a		; d=Device number
	cp	3		; 2 dispositivos somente
 IFDEF DEBUG3
	jr	nc,.noDev
 ELSE
	jr	nc,.devNotAvbl
 ENDIF
	ld	a,b
	or	a		; Device itself status?
	jp	z,.devStatus	; I'm fine, thanks
	dec	a		; Only LUN=1 are allowed
 IFDEF DEBUG3
	jr	nz,.noLun
 ELSE
	jr	nz,.devNotAvbl
 ENDIF
;.getStatus:
 IFDEF DEBUG1
	ld	a,'S'		; DEV_STATUS debug ID
	call	PRTCHAR
	ld	a,d
	add	'0'		; Device number
	call	PRTCHAR
	ld	a,'1'		; LUN number
	call	PRTCHAR
 ENDIF
	call	getWorkArea	; IX=Work area pointer
	ld	e,(ix+WRKAREA.CARDFLAGS) ; e=old LUN changed flags
	ld	a,d
	call	slctNchkCard	; Select and check the card
	call	disableSDs
	jr	c,.cardError
	jr	z,.noCard
	cp	2		; Has the card just been changed?
	jr	z,.cardChanged	; Yes, report it

	; A=1: Card hasn't changed this time. Need to check if another
	; routine has detected a change before	
	ld	a,(ix+WRKAREA.CARDFLAGS)
	or	e		; Mix with the previous flags
	and	#0C		; Crop the LUN changed flags
	rrca			; Adjust the flags position
	rrca
	and	(ix+WRKAREA.NUMSD) ; Crop the LUN changed flag for this card
	jr	nz,.cardChanged	; The card had changed before, but Nextor was still unaware
.notChanged:
 IFDEF DEBUG1
	PRTSTR	"=1"
 ENDIF
	ld	a,1		; Card is available and has not changed
	ret

.cardChanged:
	ld	a,(ix+WRKAREA.NUMSD) 
.devChanged2:
	add	a		; Adjust the flag position
	add	a
	cpl
	and	(ix+WRKAREA.CARDFLAGS)	; Clear the LUN changed flag
	ld	(ix+WRKAREA.CARDFLAGS),a
 IFDEF DEBUG1
	PRTSTR	"=2"
 ENDIF
	ld	a,2		; Card LUN is present and has changed 
	ret
.cardError:
 IFDEF DEBUG1
	ld	a,'e'
	call	PRTCHAR
	xor	a
	ret
 ENDIF
 IFDEF DEBUG3
.noLun:
	inc	a
.noDev:
	PRTSTR "S!"
	ld	e,a		; e=LUN
	ld	a,'0'
	add	d
	call	PRTCHAR
	ld	a,'0'
	add	e
	call	PRTCHAR
	xor	a
	ret
 ENDIF
.noCard:
 IFDEF DEBUG1
	ld	a,'n'
	call	PRTCHAR
 ENDIF
.devNotAvbl:
	xor	a		; Device not available 
	ret

.devChanged:
	ld	a,(ix+WRKAREA.NUMSD) 
	add	a		; Adjust the flag position
	add	a
	jr	.devChanged2

.devStatus:	; Device status has to be checked separately, otherwise
		; FDISK won't notice card changes
 IFDEF DEBUG1
	ld	a,'S'		; DEV_STATUS debug ID
	call	PRTCHAR
	ld	a,d
	add	'0'		; Device number
	call	PRTCHAR
	ld	a,'0'		; LUN number
	call	PRTCHAR
 ENDIF

	call	getWorkArea	; IX=Work area pointer
	ld	e,(ix+WRKAREA.CARDFLAGS) ; e=old LUN changed flags
	ld	a,d
	call	slctNchkCard
	call	disableSDs
	jr	c,.cardError
	jr	z,.noCard
	cp	2		; Has the card just been changed?
	jr	z,.devChanged	; Yes, report it
	; A=1: Card hasn't changed this time. Need to check if another
	; routine has detected a change before	
	ld	a,(ix+WRKAREA.CARDFLAGS)
	or	e		; Mix with the previous flags
	and	#30		; Crop the LUN changed flags
	rrca			; Adjust the flags position
	rrca
	rrca
	rrca
	and	(ix+WRKAREA.NUMSD) ; Crop the LUN changed flag for this card
	jr	nz,.devChanged	; The card had changed before, but Nextor was still unaware
	jr	.notChanged


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
;+8 (2): Number of cylinders
;+10 (1): Number of heads
;+11 (1): Number of sectors per track
;
; Number of cylinders, heads and sectors apply to hard disks only.
; For other types of device, these fields must be zero.

LUN_INFO:
	or	a		; DEV=0 is invalid
 IFDEF DEBUG3
	jr	z,.noDev
 ELSE
	jr	z,.quitError
 ENDIF
	cp	3		; Max 2 devices
 IFDEF DEBUG3
	jr	nc,.noDev
 ELSE
	jr	nc,.quitError
 ENDIF
	dec	b		; Only LUN=1 
 IFDEF DEBUG3
	jr	nz,.noLun
 ELSE
	jr	nz,.quitError
 ENDIF

 IFDEF DEBUG1
	push	af
	ld	a,'L'
	call	PRTCHAR
	pop	af
	push	af
	add	'0'		; Print the device number
	call	PRTCHAR
	ld	a,b
	add	'1'
	call	PRTCHAR
	pop	af
 ENDIF
	push	hl
	call	getWorkArea	; IX=Work area pointer
	call	slctNchkCard	; Select and check the card
	pop	hl
	or	a
 IFDEF DEBUG1
	jr	c,.cardError1	; Card error? Abort
 ELSE
	jr	c,.noMedia	; Card error? Abort
 ENDIF
	jr	nz,.devOk	; device is available
.noMedia:
 IFDEF DEBUG1
	ld	a,'n'		; LIN_INFO status: no media
	call	PRTCHAR
 ENDIF
	xor	a
	ld	b,7
.nmloop:
	ld	(hl),a		; 0=block device
	inc	hl
	djnz	.nmloop		; Fill the rest with "0=information not available"
	jr	.wflagsnCHS	; Fill the rest of the info about the device

 IFDEF DEBUG1
.cardError1:
	ld	a,'e'		; LUN_INFO status: Card error
	call	PRTCHAR
	xor	a
	ld	b,7
	jr	.nmloop
.cardError2:
	ld	a,'e'		; LUN_INFO status: Card error
	call	PRTCHAR
	jr	.quitError
 ENDIF

 IFDEF DEBUG3
.noLun:	inc	b
.noDev:
	PRTSTR	'L!'
	add	'0'
	call	PRTCHAR
	ld	a,b
	add	'0'
	call	PRTCHAR
	ld	a,1
	ret
 ENDIF

.devOk:
	xor	a
	ld	(hl),a		; 0 = block device 
	inc	hl
	ld	(hl),a		; Block size lsb 
	inc	hl
	ld	(hl),2		; Block size msb = 512
	inc	hl

	ld	de,CMD9*256+0	; Read the CSD
	call	setCxDrd
 IFDEF DEBUG1
	jr	c,.cardError2
 ELSE
	jr	c,.quitError
 ENDIF
	ex	de,hl		; de=LUN_INFO buffer+3
	ld	hl,SPIDATA

	ld	c,(hl)		; c=CSD byte 0

	ld	b,4
.skipCSDheader:
	ld	a,(hl)
	djnz	.skipCSDheader	

	ld	a,c
	and	#C0		; Crop the CSD version ID
	jr	z,.calculaCSD1
	cp	#40
	jr	z,.calculaCSD2

	ld	b,18-5
	call	flushCxD	; Flush the rest of the CID data
;	jr	.quitError	; Unknown CSD version 

.quitError:
	ld	a,1		; Return with A=1: Error
	jp	disableSDs


.savenblocks:
	pop	hl
	ld	(hl),e
	inc	hl
	ld	(hl),d
	inc	hl
	ld	(hl),c
	inc	hl
	ld	(hl),b
	inc	hl

.wflagsnCHS:
	ld	(hl),1		; flags: dispositivo R/W removivel
	inc	hl
	xor	a		; CHS = 0
	ld	(hl), a
	inc	hl
	ld	(hl), a
	inc	hl
	ld	(hl), a
 IFDEF DEBUG1
	PRTSTR	"=0"
	xor	a
 ENDIF
	jp	disableSDs	; Return with A-0: Ok, filled the buffer

; -----------------------------------
; Calculate the number of blocks from
; a CSD version 1
; Input   : none
; Output  : c:de = Number of blocks
; Modifies: af, b
; -----------------------------------
.calculaCSD1:
	push	de		; save the current Nextor buffer pointer
	ld	a,(hl)
	and	#0F		; isola READ_BL_LEN
	push	af
;	inc	hl
	ld	a,(hl)		; 2 primeiros bits de C_SIZE
	and	3
	ld	d,a
;	inc	hl
	ld	e,(hl)		; 8 bits de C_SIZE (DE contem os primeiros 10 bits de C_SIZE)
;	inc	hl
	ld	a,(hl)
	and	#C0		; 2 ultimos bits de C_SIZE
	add	a		; rotaciona a esquerda
	rl	e		; rotaciona para DE
	rl	d
	add	a		; mais uma rotacao
	rl	e		; rotaciona para DE
	rl	d
	inc	de		; agora DE contem todos os 12 bits de C_SIZE, incrementa 1
;	inc	hl
	ld	a,(hl)		; proximo byte
	and	3		; 2 bits de C_SIZE_MUL
	ld	b, a		; B contem os 2 bits de C_SIZE_MUL
;	inc	hl						
	ld	a,(hl)		; proximo byte
	and	#80		; 1 bit de C_SIZE_MUL
	add	a		; rotaciona para esquerda jogando no carry
	rl	b		; rotaciona para B
	inc	b		; agora B contem os 3 bits de C_SIZE_MUL
	inc	b		; faz B = C_SIZE_MUL + 2
	pop	af		; volta em A o READ_BL_LEN
	add	b		; A = READ_BL_LEN + (C_SIZE_MUL+2)
	ld	bc, 0
	call	.eleva2
	ld	e,d		; aqui temos 32 bits (BC DE) com o tamanho do cartao
	ld	d,c		; ignoramos os 8 ultimos bits em E, fazemos BC DE => 0B CD (divide por 256)
	ld	c,b
;	ld	b,0
	srl	c		; rotacionamos a direita o C, carry = LSB (divide por 2)
	rr	d		; rotacionamos D e E
	rr	e		; no final BC DE contem tamanho do cartao / 512 = numero de blocos

	ld	b,18-5-6
	call	flushCxD	; Flush the rest of the CSD data

	ld	b,0 		; SD cards return a 24bit number for 
 				; the number of blocks so we have to clear the
				; Nextor upper byte
	jr	.savenblocks

.eleva2:			; aqui temos: A = (READ_BL_LEN + (C_SIZE_MUL+2))
				; BC = 0
				; DE = C_SIZE
	sla	e		; rotacionamos C_SIZE por 'A' vezes
	rl	d
	rl	c
	rl	b
	dec	a		; subtraimos 1
	jr	nz,.eleva2
	ret			; em BC DE temos o tamanho do cartao (bytes) em 32 bits


; -----------------------------------
; Calculate the number of blocks from
; a CSD version 2
; Input   : none
; Output  : c:de = Number of blocks
; Modifies: af, b
; -----------------------------------
.calculaCSD2:
	push	de		; save the current Nextor buffer pointer
;	inc	hl		; HL ja aponta para BCSD+5, fazer HL apontar para BCSD+7
;	inc	hl
	ld	a,(hl)		; Discard 2 bytes
	ld	a,(hl)

	ld	a,(hl)
	and	#3F
	ld	c,a
;	inc	hl
	ld	d,(hl)
;	inc	hl
	ld	e,(hl)
	call	.inc32		; soma 1
	call	.desloca32	; multiplica por 512
	call	.rotaciona24	; multiplica por 2

	push	bc
	ld	b,18-5-5
	call	flushCxD	; Flush the rest of the CSD data
	pop	bc		; SDHC/SDXC cards have a 32bit size
	jp	.savenblocks

.inc32:
	inc	e
	ret	nz
	inc	d
	ret	nz
	inc	c
	ret	nz
	inc	b
	ret

.desloca32:
	ld	b, c
	ld	c, d
	ld	d, e
	ld	e, 0
.rotaciona24:
	sla	d
	rl	c
	rl	b
	ret




;=====
;=====  END of DEVICE-BASED specific routines
;=====

;------------------------------------------------
; Rotinas auxiliares
;------------------------------------------------

;------------------------------------------------
; Get the SLTWRK pointer 
; Output: 
;           IX = SLTWRK pointer
; Modifies: AF'
;------------------------------------------------
getWorkArea:
	push	af
	xor	a		; A=0: GWORK current slot
	ex	af,af'
	xor	a		; A=0: BANK0 
	ld	ix,GWORK
	call	CALBNK
	pop	af
	ret

;------------------------------------------------
; Marcar bit de erro nas flags
; Destroi AF, C
;------------------------------------------------
marcaErroCartao:
	ld	c, (ix+WRKAREA.NUMSD)		; cartao atual (1 ou 2)
	ld	a, (ix+WRKAREA.CARDFLAGS)	; marcar erro
	or	c
	ld	(ix+WRKAREA.CARDFLAGS), a
	ret

;------------------------------------------------
; Testar se cartao atual esta protegido contra
; gravacao, A=0 se protegido
; Destroi AF, C
;------------------------------------------------
testaWP:
	ld	a, (ix+WRKAREA.NUMSD)	; cartao atual (1 ou 2)
	ld	(SPICTRL), a
	ld	a, (SPISTATUS)	; testar se cartao esta protegido
;	call	disableSDs
	and	#04
	ret			; se A for 0 cartao esta protegido



;------------------------------------------------
; Minhas funcoes para cartao SD
;------------------------------------------------

;------------------------------------------------
; Processo de inicializacao e deteccao do cartao.
; Detecta se cartao responde, qual versao (SDV1
; ou SDV2), faz a leitura do CSD e CID e calcula
; o numero de blocos do cartao, colocando o CID
; e total de blocos no buffer correto dependendo
; do cartao 1 ou 2.
; Retorna erro no carry. Se for 0 indica deteccao
; com sucesso.
; Destroi todos os registradores
; Input   : (ix+WRKAREA.NUMSD)
; Output  : (ix+WRKAREA.CARDFLAGS)
;           Cy set in case of error
;           E=DEV_RW error code in case of error
; Modifies: AF, BC, DE, HL
;------------------------------------------------
detectaCartao:
 IFDEF DEBUG1
	PRTSTR	"<detect card "
	ld	a,(ix+WRKAREA.NUMSD)
	add	'0'
	call	PRTCHAR
	PRTSTR	" v"
 ENDIF
	call	iniciaSD		; Initialize this SD card
	ld	e,ENRDY
	jp	c,.quitError		; This card has an error: quit
	call	detSDversion		; Check if this is a V2 card
 IFDEF DEBUG1
	push	af
	add	'0'
	call	PRTCHAR
	pop	af
 ENDIF
	jr	c,.incompCardV2		; Incompatible card
	ld	a,CMD58			; Read OCR
	ld	de,0
	call	SD_SEND_CMD_2_ARGS_GET_R3
	jr	c,.incompCardCMD58	; Incompatible card

	; Set the card version on WRKAREA.CARDFLAGS
	ld	a,(ix+WRKAREA.NUMSD)
	rrca				; Adjust the bits position
	rrca
	ld	e,a			; e=b7,b6: Card number normal mask
	cpl
	ld	c,a			; c=b7,b6: Card number reverse mask
	ld	a,b			; Get the part of the OCR we need 
	and	#40			; Crop the card version bit
	bit	7,e			; Is the card-2 slot selected?
	jr	z,.saveSDver		; No, skip
	add	a			; Adjust the version bit position
.saveSDver:
	ld	b,a			; b=card version flag for this card slot
	ld	a,(ix+WRKAREA.CARDFLAGS)
	and	c			; Clear the previous version flag
	or	b
	ld	(ix+WRKAREA.CARDFLAGS),a
	and	e			; Is this card V1?
	call	z,mudarTamanhoBlocoPara512	; Yes, set the block size to 512
	jr	c,.incompCardBlk512		; Incompatible card

	; Set the rest of the WRKAREA.CARDFLAGS
	ld	a,(ix+WRKAREA.NUMSD)
	ld	c,a			; c=Card OR mask
	cpl
	ld	b,a			; b=Card AND mask
	ld	a,(ix+WRKAREA.CARDFLAGS)
	and	b			; Clear the error flag
	sla	c			; Adjust to the flag position
	sla	c
	or	c			; Flag that LUN has changed
	sla	c			; Adjust to the flag position
	sla	c
	or	c			; Flag that DEV has changed
	ld	(ix+WRKAREA.CARDFLAGS),a
 IFDEF DEBUG1
	PRTSTR	" ok>"
 ENDIF
	ret

 IFNDEF DEBUG1
.incompCardV2:			; Incompatible card v2 error
.incompCardCMD58:		; Incompatible card CMD58 error
.incompCardBlk512:		; Incompatible card setBlk512 error
	ld	e,ENCOMP
 ELSE
.incompCardV2:			; Incompatible card v2 error
	PRTSTR	" iv2>"
	ld	e,ENCOMP
	jr	.quitError2
.incompCardCMD58:		; Incompatible card CMD58 error
	PRTSTR	" icmd58>"
	ld	e,ENCOMP
	jr	.quitError2
.incompCardBlk512:		; Incompatible card setBlk512 error
	PRTSTR	" iblk512>"
	ld	e,ENCOMP
	jr	.quitError2
 ENDIF

.quitError:
 IFDEF DEBUG1
	PRTSTR	" e>"
.quitError2:
 ENDIF
	call	disableSDs
	ld	a,(ix+WRKAREA.CARDFLAGS)
	or	(ix+WRKAREA.NUMSD)	; Flag that this card has an error
	scf
	ret




; ------------------------------------------------
; Setar o tamanho do bloco para 512 se o cartao
; for SDV1
; ------------------------------------------------
mudarTamanhoBlocoPara512:
 IFDEF DEBUG2
	PRTSTR	"<setBlk512>"
 ENDIF
	ld	a, CMD16
	ld	bc, 0
	ld	de, 512
	jp	SD_SEND_CMD_GET_ERROR

; ------------------------------------------------
; Detects the SD card version 
; Output: A=Card version
;           0: Unknown
;           1: SD V1
;           2: SD V2
;           3: MMC V3
;         Cy set on error
; ------------------------------------------------
detSDversion:
 IFDEF DEBUG2
	PRTSTR	"<detSDversion"
 ENDIF
	ld	a, CMD8
	ld	de, #1AA
	call	SD_SEND_CMD_2_ARGS_GET_R3
	jr	c,.v1Card
	ld	a,d
	and	1
	ld	d,a
	ld	hl,#1AA
	sbc	hl,de			; Lower 12-bit were 1AAh?
 IFDEF DEBUG2
	ld	a,0
	jr	nz,.incompat
 ELSE
	scf
	ret	nz			; No, quit with error
 ENDIF
;.v2Card:
	ld	hl,SD_SEND_ACMD41
	call	.init
	ld	a,2
 IFDEF DEBUG2
	jr	c,.incompat
	PRTSTR	" 2>"
 ENDIF
	ret				; SD V2, if nc


.v1Card:
	ld	hl,SD_SEND_ACMD41
	call	.init
	ld	a,1
 IFDEF DEBUG2
	jr	c,.v3MMC
	PRTSTR	" 1>"
	ret
 ELSE
	ret	nc			; SD V1, if ACMD41 was accepted
 ENDIF

.v3MMC:
	ld	hl,SD_SEND_CMD1
	call	.init
	ld	a,3
 IFDEF DEBUG2
	jr	c,.incompat
	PRTSTR	" 3>"
 ELSE
	ret				; MMC V3, if nc
 ENDIF


.init:
	ld	bc, LOW DETTIMEOUT * 256 + HIGH DETTIMEOUT
.loop1:
	ld	a,255			; 2.6mS 
	ld	(TIMERREG),a
.loop2:
	push	bc
	call	.jumpHL		; chamar rotina correta em HL
	pop	bc
	ret	z
	ld	a,(TIMERREG)
	or	a
	jr	nz,.loop2
	djnz	.loop1
	dec	c
	jr	nz,.loop1
	scf
	ret
.jumpHL:
	jp	(hl)		; chamar rotina correta em HL



 IFDEF DEBUG2
.incompat:
	push	af
	push	af
	ld	a,' '
	call	PRTCHAR
	pop	af
	add	'0'
	call	PRTCHAR
	pop	af
	PRTSTR	"e>"
	scf
	ret
 ENDIF


; ------------------------------------------------
; Reads the CxD (CID, CSD) and skips data until it points
; to the requested field
; Input   : D=SD command
;           E=requested CID field
; Modifies: AF, BC, DE
; ------------------------------------------------
setCxDrd:
;	call	setaSDAtual
	push	de
	ld	a,d	; get SD command
	call	SD_SEND_CMD_NO_ARGS
	pop	de
	ret	c
	call	WAIT_RESP_FE
	ret	c

	ld	a,e
	or	a
	ret	z
	ld	b,e
.skipfields:
	ld	a,(SPIDATA)
	djnz	.skipfields
	ret


; ------------------------------------------------
; Flush the rest of the CxD (CID, CSD) data
; and disable the SD card slots
; Input   : B=Number of bytes to flush 
; Modifies: A
; ------------------------------------------------
flushCxD:
	ld	a,(SPIDATA)
	djnz	flushCxD
	jp	disableSDs





; ------------------------------------------------
; Algoritmo para inicializar um cartao SD
; Destroi AF, B, DE
; ------------------------------------------------
iniciaSD:
	ld	a, (ix+WRKAREA.NUMSD)
	ld	(SPICTRL), a
	call	disableSDs

	ld	b,10		; enviar 80 pulsos de clock com cartao desabilitado
	ld	a,#FF		; manter MOSI em 1
enviaClocksInicio:
	ld	(SPIDATA),a
	djnz	enviaClocksInicio
;	call	setaSDAtual	; ativar cartao atual
;	jp	c,disableSDs	; Quit on error

	ld	a,(SPIDATA)		; Dummy read
	ld	a,(ix+WRKAREA.NUMSD)
	ld	(SPICTRL),a

;	call	disableSDs

	ld	b,8		; 8 tentativas para CMD0
SD_SEND_CMD0:
	ld	a, CMD0		; primeiro comando: CMD0
	ld	de, 0
	push	bc
	call	SD_SEND_CMD_2_ARGS_TEST_BUSY
	pop	bc
	ret	nc			; retorna se cartao respondeu ao CMD0
	djnz	SD_SEND_CMD0
	scf			; cartao nao respondeu ao CMD0, informar erro
	; fall through

; ------------------------------------------------
; Desabilitar (de-selecionar) todos os cartoes
; Nao destroi registradores
; ------------------------------------------------
disableSDs:
	push	af
	xor	a
	ld	(SPICTRL), a
	dec	a
	ld	(SPIDATA), a		; Dummy write to release DO
	pop	af
	ret


; ------------------------------------------------
; Get the selected card version
; Input   : (WRKAREA.NUMSD)
; Output  : A=Card version (0=V1, NZ=V2)
;         : Flag Z will be updated accordingly
; Modifies: none
; ------------------------------------------------
getCardVer:
	ld	a,(ix+WRKAREA.CARDFLAGS)
	rlca				; Adjust the flag position
	rlca
	and	(ix+WRKAREA.NUMSD)	; Crop this card version
	ret

; ------------------------------------------------
; Enviar comando ACMD41
; ------------------------------------------------
SD_SEND_ACMD41:
	ld	a,CMD55
	call	SD_SEND_CMD_NO_ARGS
	ld	a,ACMD41
	ld	bc,#4000
	ld	d,c
	ld	e,c
	jr	SD_SEND_CMD_GET_ERROR

; ------------------------------------------------
; Enviar CMD1 para cartao. Carry indica erro
; Destroi AF, BC, DE
; ------------------------------------------------
SD_SEND_CMD1:
	ld	a,CMD1
SD_SEND_CMD_NO_ARGS:
	ld	bc,0
	ld	d, b
	ld	e, c
SD_SEND_CMD_GET_ERROR:		; Send command with BC:DE as its parameter
	call	SD_SEND_CMD
	jr	c,disableSDs	; Quit on error
	or	a
	ret	z			; se A=0 nao houve erro, retornar
setaErro:
	scf
	jr	disableSDs

; ------------------------------------------------
; Enviar comando em A com 2 bytes de parametros
; em DE e testar retorno BUSY
; Retorna em A a resposta do cartao
; Destroi AF, BC
; ------------------------------------------------
SD_SEND_CMD_2_ARGS_TEST_BUSY:
	ld	bc,0
	call	SD_SEND_CMD
	ld	b, a
	and	#FE		; Test all error flags except on bit0 
	ld	a, b
	jr	nz,setaErro	; Abort if there are any errors 
	ret

; ------------------------------------------------
; Enviar comando em A com 2 bytes de parametros
; em DE e ler resposta do tipo R3 em BC DE
; Retorna em A a resposta do cartao
; Destroi AF, BC, DE, HL
; Output: BC:DE return code
; ------------------------------------------------
SD_SEND_CMD_2_ARGS_GET_R3:
	call	SD_SEND_CMD_2_ARGS_TEST_BUSY
	push	af
	call	WAIT_RESP_NO_FF
	ld	h, a
	call	WAIT_RESP_NO_FF
	ld	l, a
	call	WAIT_RESP_NO_FF
	ld	d, a
	call	WAIT_RESP_NO_FF
	ld	e, a
	ld	b, h
	ld	c, l
	pop	af
	ret


; ------------------------------------------------
; Enviar comando em A com 4 bytes de parametros
; em BC:DE e enviar CRC correto se for CMD0 ou 
; CMD8 e aguardar processamento do cartao
; Output  : A=0 if there was no error
; Modifies:  AF, B
; ------------------------------------------------
SD_SEND_CMD:
	call	setaSDAtual
 IFDEF PARALLELCARD
	push	af
	push	bc
	call	WAIT_RESP_NO_00
	pop	bc
	pop	af
 ENDIF
	ld	(SPIDATA), a
	push	hl
	ld	l,b		; Swap the endianess
	ld	h,c
	ld	(SPIDATA),hl
	ld	l,d		; Swap the endianess
	ld	h,e
	ld	(SPIDATA),hl
	pop	hl
	cp	CMD0
	ld	b,#95		; CMD0 CRC
	jr	z,.enviaCRC
	cp	CMD8
	ld	b,#87		; CMD8 CRC
	jr	z,.enviaCRC

	; Disabled these checksums because they caused problems
	; with some SDV1 cards. It's said that some Toshiba cards
	; need them, but I don't have any to test.
	; In case this need to be enabled someday, probably the best
	; approach will be to send the appropriate ACMD41 checksum to
	; the respective card version
;	cp	CMD55
;	ld	b,#65		; CMD55 CRC
;	jr	z,.enviaCRC
;	cp	ACMD41
;	ld	b,#77		; ACMD41 CRC  (E5h for non-SDHC/SDXC cards)
;	jr	z,.enviaCRC
	ld	b,#FF		; dummy CRC
.enviaCRC:
	ld	a,b
	ld	(SPIDATA),a
	jr	WAIT_RESP_NO_FF


; ------------------------------------------------
; Esperar que resposta do cartao seja #FE
; Destroi AF, BC
; ------------------------------------------------
WAIT_RESP_FE:
	ld	bc,CMDTIMEOUT*256+#FE
.loop1:
	ld	a,255		; 2.6mS 
	ld	(TIMERREG),a

.loop2:	ld	a,(SPIDATA)
	cp	c		; resposta é #FE ?
	ret	z		; sim, retornamos com carry=0
	ld	a,(TIMERREG)
	or	a
	jr	nz,.loop2
	djnz	.loop1

	xor	a		; No error flags and Cy set = timeout
	scf
	ret

; ------------------------------------------------
; Esperar que resposta do cartao seja diferente
; de #FF
; Destroi AF, BC
; ------------------------------------------------
WAIT_RESP_NO_FF:
	ld	bc,CMDTIMEOUT*256+#FF
.loop1:
	ld	a,255		; 2.6mS 
	ld	(TIMERREG),a

.loop2:	ld	a,(SPIDATA)
	cp	c		; A=#FF?	
	ccf
	ret	nc		; No, quit
	ld	a,(TIMERREG)
	or	a
	jr	nz,.loop2
	djnz	.loop1

	xor	a
	scf			; Error: timeout
	ret

; ------------------------------------------------
; Esperar que resposta do cartao seja diferente
; de #00
; Destroi A, BC
; ------------------------------------------------
WAIT_RESP_NO_00:
	ld	bc, LOW READYTIMEOUT * 256 + HIGH READYTIMEOUT
.loop1:	ld	a,255			; 2.6mS 
	ld	(TIMERREG),a

.loop2:	ld	a, (SPIDATA)
	or	a
	ret	nz		; se resposta for <> #00, sai
	ld	a,(TIMERREG)
	or	a
	jr	nz,.loop2
	djnz	.loop1
	dec	c
	jr	nz,.loop1

	xor	a		; No error flags and Cy set = timeout
	scf
	ret

; ------------------------------------------------
; Sets the requested card slot
; Input: Target card slot
; Output: Cy = No SD card is present
;         A: 0=The same card is still present
;            1=The card was changed since the last check
; ------------------------------------------------
setaSDAtual:
	push	af
 IFNDEF PARALLELCARD
	xor	a
	ld	(SPICTRL),a
	ld	a,(SPIDATA)		; Dummy read
 ENDIF
	ld	a,(ix+WRKAREA.NUMSD)
	ld	(SPICTRL),a

; IFDEF PARALLELCARD
;	push	bc
;	call	WAIT_RESP_NO_00
;	pop	bc
;	jr	c,.error
; ENDIF
	pop	af
;	or	a		; Clear Cy
	ret

; IFDEF PARALLELCARD
;.error: pop	af		; Flush the stack
;	xor	a		; No error flags and Cy set = timeout
;	scf
;	ret
; ENDIF



; ------------------------------------------------
; Grava um bloco de 512 bytes no cartao
; HL = aponta para o inicio dos dados
; BC:DE = contem o numero do bloco (BCDE = 32 bits)
; Modifies:  AF, BC, DE, HL, IXL
; ------------------------------------------------
GravarBloco:
	call	getCardVer
	call	z,blocoParaByte		; se for SDV1 coverter blocos para bytes
;	call	setaSDAtual	; selecionar cartao atual
	ld	a, (ix+WRKAREA.NUMBLOCKS)	; Get the number of blocka 
	ld	iyh,a		; iyh=Number of blocks
	dec	a
	jp	z,.umBloco	; somente um bloco, gravar usando CMD24

; multiplos blocos
 IFDEF DEBUG1
	push	af
	ld	a,'m'
	call	PRTCHAR
	pop	af
 ENDIF
	push	bc
	push	de
	ld	a, CMD55	; Multiplos blocos, mandar ACMD23 com total de blocos
	call	SD_SEND_CMD_NO_ARGS
	ld	a, ACMD23
	ld	bc, 0
	ld	d, c
	ld	e, (ix+WRKAREA.NUMBLOCKS)	; parametro = total de blocos a gravar
	call	SD_SEND_CMD_GET_ERROR
	pop	de
	pop	bc
	jp	c,R1ERROR	; erro no ACMD23
 IFDEF DEBUG1
	call	PRTDOT
 ENDIF
	ld	a, CMD25	; comando CMD25 = write multiple blocks
	call	SD_SEND_CMD_GET_ERROR
	jp	c,R1ERROR	; erro
 IFDEF DEBUG1
	call	PRTDOT
 ENDIF
.loop:
	ld	a, #FC		; mandar #FC para indicar que os proximos dados sao
	ld	(SPIDATA),a	; dados para gravacao

	ld	de,SPIDATA
	call	RUN_HLPR

;	ld	a,#FF		; envia dummy CRC
;	ld	(SPIDATA),a	; Can't be done with ld (SPIDATA),de. It's too fast
;	ld	(SPIDATA),a
	ld	d,e		; CRC1=CRC2
	ld	(SPIDATA),de	; Send a dummy CRC

	call	WAIT_RESP_NO_FF	; esperar cartao
	and	#1F		; Crop the "Data Response"
	cp	5		; Data accepted?
	scf
	jp	nz,DRERROR.multi	; Data response error 
	call	WAIT_RESP_NO_00	; wait for the card
	jr	c,R1ERROR
 IFDEF DEBUG1
	call	PRTDASH
 ENDIF
	dec	iyh		; Next block
	jp	nz,.loop

	ld	hl,(SPIDATA)	; acabou os blocos, fazer 2 dummy reads
	ld	a, #FD		; enviar #FD para informar ao cartao que acabou os dados
	ld	(SPIDATA),a
	ld	hl,(SPIDATA)	; 2 dummy reads
 IFDEF PARALLELCARD
	ld	a,(SPIDATA)	; dummy status read
 ELSE
	call	WAIT_RESP_NO_00	; wait for the card
 ENDIF
	jr	.end		; CMD25 finished without error 

.umBloco:
 IFDEF DEBUG1
	push	af
	ld	a,'s'
	call	PRTCHAR
	pop	af
 ENDIF
	ld	a, CMD24	; gravar somente um bloco com comando CMD24 = Write Single Block
	call	SD_SEND_CMD_GET_ERROR
	jr	c,R1ERROR	; erro

 IFDEF DEBUG1
	call	PRTDOT
 ENDIF
	ld	a, #FE		; mandar #FE para indicar que vamos mandar dados para gravacao
	ld	(SPIDATA),a

	ld	de,SPIDATA
	call	RUN_HLPR

;	ld	a,#FF		; envia dummy CRC
;	ld	(SPIDATA),a	; Can't be done with ld (SPIDATA),de. It's too fast
;	ld	(SPIDATA),a
	ld	h,l		; CRC1=CRC2
	ld	(SPIDATA),hl	; Send a dummy CRC

	call	WAIT_RESP_NO_FF	; esperar cartao
	and	#1F		; Crop the "Data Response"
	cp	5		; Data accepted?
	scf
	jr	nz,DRERROR.single	; Data response error 
 IFDEF DEBUG1
	call	PRTDASH
 ENDIF
 IFDEF PARALLELCARD
	ld	a,(SPIDATA)	; dummy status read
 ELSE
	call	WAIT_RESP_NO_00	; wait for the card
 ENDIF
.end:
	xor	a		; No errors to report
	jp	disableSDs



R1ERROR:	; R1 response error
	push	af
	ld	a,CMD12		; Abort the current command 
	call	SD_SEND_CMD_NO_ARGS
	pop	af
.noabort:
	rrca			; In idle state?
	ld	e,ENRDY
	ret	c
	rrca			; Erase reset?
	ld	e,EWRERR
	ret	c
	rrca			; Illegal command?
	ld	e,ENCOMP
	ret	c
	rrca			; Communication CRC error?
	ld	e,EDATA
	ret	c
	rrca			; Erase sequence error?
	ld	e,EWRERR
	ret	c
	rrca			; Address error?
	ld	e,ESEEK
	ret	c
	rrca			; Parameter error?
	ld	e,ERNF
	ret	c
.timeout:
 IFDEF DEBUG1
	ld	a,'t'
	call	PRTCHAR
 ENDIF
	ld	e,ENRDY		; Timeout error
	scf
	jp	disableSDs

DRERROR:	; Data response error
.multi:
	push	af
	ld	a,CMD12		; Abort the current command 
	call	SD_SEND_CMD_NO_ARGS
	pop	af
.single:
	cp	#0B		; CRC error?
	ld	e,EDATA
	scf
	jp	z,disableSDs
	cp	#0D		; Data write error?
 IFDEF DEBUG1
	call	nz,PRTHEX
 ENDIF
	ld	e,EWRERR
	scf
	jp	nz,disableSDs	; No, quit with generic write  error
	ld	a,CMD13		; Ask for 
	call	SD_SEND_CMD_NO_ARGS
	call	WAIT_RESP_NO_FF	; Get R2-part1 (R1)
	ld	h,a
	call	WAIT_RESP_NO_FF ; Get R2-part2
	ld	l,a
	call	disableSDs
	ld	a,h
	call	R1ERROR.noabort
	ld	a,e
	cp	ENRDY		; fake timeout just means that no R1 bits were set
	scf
	ret	nz		; Quit if any other error
	; Process R2-part2 error bits
	ld	a,l
	rrca			; Card is locked?
	ld	e,EWPROT
	ret	c
	rrca			; wp erase skip|lock/unlock cmd failed?
	ret	c
	rrca			; General Unknown error?
	ld	e,EDISK
	ret	c
	rrca			; Internal controller error?
	ld	e,ESEEK
	ret	c
	rrca			; Card ECC failed?
	ld	e,EDATA
	ret	c
	rrca			; Write protect violation?
	ld	e,EWPROT
	ret	c
	rrca			; Erase param?
	ld	e,ERNF
	ret	c
.timeout:
 IFDEF DEBUG1
	ld	a,'t'
	call	PRTCHAR
 ENDIF
	ld	e,ENRDY		; Timeout error
	scf
	ret



; ------------------------------------------------
; Ler um bloco de 512 bytes do cartao
; HL =  aponta para o inicio dos dados
; BC:DE = contem o numero do bloco (BCDE = 32 bits)
; Destroi AF, BC, DE, HL, IXL
; ------------------------------------------------
LerBloco:
	call	getCardVer
	call	z,blocoParaByte	; se for SDV1 coverter blocos para bytes
;	call	setaSDAtual

	ld	a, (ix+WRKAREA.NUMBLOCKS)	; Get the number of blocka 
	ld	iyh,a		; iyh=Number of blocks
	dec	a
	jp	z,.umBloco	; only one block

; multiplos blocos
 IFDEF DEBUG2
	ld	a,'m'
	call	PRTCHAR
 ENDIF
	ld	a, CMD18	; ler multiplos blocos com CMD18 = Read Multiple Blocks
	call	SD_SEND_CMD_GET_ERROR
	jr	c,.r1error
	ex	de,hl		; de=Destination address
.loop:
	call	WAIT_RESP_FE
	jr	c,.r1error

	ld	hl,SPIDATA
	call	RUN_HLPR
	ld	hl,(SPIDATA)	; Discard the CRC
 IFDEF DEBUG2
	call	PRTDOT
 ENDIF
	dec	iyh
	jp	nz,.loop
	ld	a, CMD12	; acabou os blocos, mandar CMD12 para cancelar leitura
	call	SD_SEND_CMD_NO_ARGS
	jr	.end

.umBloco:
 IFDEF DEBUG2
	ld	a,'s'
	call	PRTCHAR
 ENDIF
	ld	a, CMD17	; ler somente um bloco com CMD17 = Read Single Block
	call	SD_SEND_CMD_GET_ERROR
	jr	c,.r1error

	call	WAIT_RESP_FE
	jr	c,.r1error
	ex	de,hl

	ld	hl,SPIDATA
	call	RUN_HLPR
	ld	hl,(SPIDATA)	; Discard the CRC 
 IFDEF DEBUG2
	call	PRTDOT
 ENDIF

.end:	xor	a		; No errors to report
	jp	disableSDs
.r1error:
	jp	R1ERROR



; ------------------------------------------------
slctNchkCard:
; Selects a card slot, check for the hardware card change flag
; and, if it changed, calls the detection of the new card
;
; Obs: It was done this way for software robustness.
; This way we don't have to rely that Nextor will
; behave in any expected way, like calling DEV_STATUS
; before any disk operation.
; 
; Input   : A = Card slot (1 or 2)
; Output  : Cy set in case of a defective card
;           A = 0: No card is present (Z is also set)
;               1: A card is present and has not changed since the last check 
;               2: A card is present and has changed since the last check 
;           E = DEV_RW error code in case of error
; Modifies:  BC, DE, HL
; ------------------------------------------------
	; Sanity check
	cp	3
	jr	nc,.nocard
	and	a
	jr	z,.nocard
	;
	ld	(ix+WRKAREA.NUMSD),a
	ld	(SPICTRL),a
;	call	setaSDAtual
;	ret	c

	ld	a,(SPISTATUS)		; Get this card-slot status
	bit	SD_PRESENT,a		; Any card here?
	jr	nz,.nocard		; Report that there's no card here
	and	SD_M_DSKCHG		; Has the card changed since last time?
	jr	nz,.cardChanged		; Card has changed, detect it
.tsterror:
	ld	a,(ix+WRKAREA.CARDFLAGS) ; check if this card had an error
	and	(ix+WRKAREA.NUMSD)
	call	nz,detectaCartao	; Card had error, force re-detection
	ld	a,1			; A=1: same card
	jp	c,disableSDs		; Quit on error
;.sameCard:
	ld	a,(ix+WRKAREA.NUMSD)
	cpl
	rlca
	rlca
	ld	b,a			; b=LUN changed clear mask
	rlca
	rlca
	and	b			; a=DEV/LUN changed clear mask
	and	(ix+WRKAREA.CARDFLAGS)	; clear the disk change bits
	ld	(ix+WRKAREA.CARDFLAGS),a
	ld	a,1			; A=1: same card
	or	a			; Set NZ
	ret

.cardChanged:
	call	detectaCartao		; Detect the new card 
	ld	a,2			; A=2: card changed
	jp	c,disableSDs		; Quit on error
	or	a			; Set NZ
	ret

.nocard:
	xor	a
	jp	disableSDs

; ------------------------------------------------
; Converte blocos para bytes. Na pratica faz
; BC DE = (BC DE) * 512
; ------------------------------------------------
blocoParaByte:
 IFDEF DEBUG2
	PRTSTR "<Blk2Byte>"
 ENDIF
	ld	b, c
	ld	c, d
	ld	d, e
	ld	e, 0
	sla	d
	rl	c
	rl	b
	ret

; ------------------------------------------------
; Funcoes utilitarias
; ------------------------------------------------


; ------------------------------------------------
printString:
; Prints an ASCII string that has the last bit7 set
; Input   : DE = Pointer to the string
; Modifies: AF, DE, EI 
; ------------------------------------------------
	ld	a,(de)
	bit	7,a
	res	7,a
	call	CHPUT
	ret	nz
	inc	de
	jr	printString


; ------------------------------------------------
HexToAscii:
; Converts the byte in A to a text string in hexa
; on the buffer pointed by HL
; Modifies: AF, C, HL
; ------------------------------------------------
	ld	c, a
	rra
	rra
	rra
	rra
	call	.conv
	ld  	a, c
.conv:
	and	#0F
	add	#90
	daa
	adc	#40
	daa
	ld	(hl),a
	inc	hl
	ret

; ------------------------------------------------
; Get the Maker Name from the Maker ID
; Output: DE = Pointer to Manufacturer name
; ------------------------------------------------
getMakerName:
; IF (tblMakerIndex-tblMakerIndex.end < 512)
	cp	(tblMakerIndex.end-tblMakerIndex)/2+1	; > last ID?
	jr	nc,.Unknown		; Yes->Unknown maker
; ENDIF
	push	hl
	ld	l,a
	ld	h,0
	ld	de,tblMakerIndex
	add	hl,hl
	add	hl,de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	pop	hl
	ret

; IF (tblMakerIndex-tblMakerIndex.end < 512)
.Unknown:
	ld	de,tblMakerNames.idUkn
	ret
; ENDIF


tblMakerIndex:
	dw	tblMakerNames.idUkn
	dw	tblMakerNames.idx01
	dw	tblMakerNames.idx02
	dw	tblMakerNames.idx03
	dw	tblMakerNames.idx04
	dw	tblMakerNames.idUkn
	dw	tblMakerNames.idx06
 REPT #10-#06
	dw	tblMakerNames.idUkn
 ENDM
	dw	tblMakerNames.idx11
	dw	tblMakerNames.idUkn
	dw	tblMakerNames.idx13
	dw	tblMakerNames.idUkn
	dw	tblMakerNames.idUkn
	dw	tblMakerNames.idUkn
	dw	tblMakerNames.idUkn
	dw	tblMakerNames.idx18
	dw	tblMakerNames.idUkn
	dw	tblMakerNames.idx1A
	dw	tblMakerNames.idx1B
	dw	tblMakerNames.idx1C
	dw	tblMakerNames.idx1D
	dw	tblMakerNames.idUkn
	dw	tblMakerNames.idx1F
 REPT #26-#1F
	dw	tblMakerNames.idUkn
 ENDM
	dw	tblMakerNames.idx27
	dw	tblMakerNames.idx28
 REPT #30-#28
	dw	tblMakerNames.idUkn
 ENDM
	dw	tblMakerNames.idx31
 REPT #40-#31
	dw	tblMakerNames.idUkn
 ENDM
	dw	tblMakerNames.idx41
 REPT #72-#41
	dw	tblMakerNames.idUkn
 ENDM
	dw	tblMakerNames.idx73
	dw	tblMakerNames.idx74
	dw	tblMakerNames.idUkn
	dw	tblMakerNames.idx76
 REPT #81-#76
	dw	tblMakerNames.idUkn
 ENDM
	dw	tblMakerNames.idx82
 REPT #88-#82
	dw	tblMakerNames.idUkn
 ENDM
	dw	tblMakerNames.idx89
 REPT #9B-#89
	dw	tblMakerNames.idUkn
 ENDM
	dw	tblMakerNames.idx9C
	dw	tblMakerNames.end
.end:



tblMakerNames:
.idUkn:	DEFB 	"Unknow","n" OR #80
.idx01:	DEFB 	"Panasoni","c" OR #80
.idx02:	DEFB 	"Toshib","a" OR #80
.idx03: DEFB 	"SanDis","k" OR #80
.idx04: DEFB 	"SMI-","S" OR #80
.idx06: DEFB 	"Renesa","s" OR #80
.idx11: DEFB 	"Dane-Ele","c" OR #80
.idx13: DEFB 	"KingMa","x" OR #80
.idx18: DEFB 	"Infineo","n" OR #80
.idx1A: DEFB 	"PQ","I" OR #80
.idx1B: DEFB 	"Samsun","g" OR #80
.idx1C: DEFB 	"Transcen","d" OR #80
.idx1D: DEFB 	"ADAT","A" OR #80
.idx1F: DEFB 	"SiliconPowe","r" OR #80
.idx27: DEFB 	"Phiso","n" OR #80
.idx28: DEFB 	"Lexa","r" OR #80
.idx31: DEFB 	"Silicon Powe","r" OR #80
.idx41: DEFB 	"Kingsto","n" OR #80
.idx73: DEFB 	"SilverH","T" OR #80
.idx74: DEFB 	"Transcen","d" OR #80
.idx76: DEFB 	"Patrio","t" OR #80
.idx82: DEFB 	"Son","y" OR #80
.idx89: DEFB 	"L.Dat","a" OR #80
.idx9C: DEFB 	"Angelbir","d" OR #80
.end:


; ------------------------------------------------
; Restore screen parameters on MSX>=2 if they're
; not set yet
; ------------------------------------------------
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
	ld	c,#23			; Block-2, R#3
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
	add	a
	add	a
	add	a
	add	a
	or	b
	ld	b,a
	ld	a,(LINLEN)
	cp	b
	ret	z
.restore:
	xor	a		; Don't displat the function keys
	ld	ix,SDFSCR
	jp	EXTROM

; ------------------------------------------------
; Check if the STOP key was signaled on DRV_INIT
; ------------------------------------------------
INICHKSTOP:
	ld	a,(INTFLG)
	cp	4			; Was STOP pressed?
	ret	nz			; No, quit as fast as possible 

	; Handle STOP to pause and read messages, and ask for the copyright info
	ld	de,strBootpaused
	call	printString
.wait1:	ld	a,7
	call	SNSMAT
	and	#10			; Is STOP still pressed?
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
	ld	de,strCopyright
	jp	printString


; ------------------------------------------------
STR_SANITIZE:
; Sanitize a string before it is sent to Nextor
; Input   : HL = Pointer to the string
;            B = string size
; Output  :  B = string size
;         : DE = Pointer to the end of the buffer
; Modifies:  A, C
; ------------------------------------------------
	ld	c,0			; Flag to test if the string only has spaces
	push	bc
	push	hl
.loop1:
	ld	a,(hl)
	cp	32
	jr	z,.next
	ld	c,1
	call	c,.invChar
	cp	127
	call	nc,.invChar
.next:	inc	hl
	djnz	.loop1
	pop	hl
	ld	a,c
	pop	bc
	or	a			; Has chars other than only spaces?
	ret	nz			; Yes, quit
	; Copy <null> to the buffer
	push	de
	ex	de,hl			; de=Pointer to the string
	ld	hl,nullTxt
	ld	bc,nullTxt.end-nullTxt
	ldir
	pop	bc			; Discard the old pointe position
	ld	b,nullTxt.end-nullTxt
	ret

.invChar:				; Replace an invalid char
	ld	(hl),'_'
	ret


; ------------------------------------------------
INSTR800HLP:
; Install the R800 data transfer helper routine on extra WorkArea 
; ------------------------------------------------
	ld	a,(MSXVER)
	cp	3		; MSX Turbo-R?
	ret	c		; No, return
	call	GTR800LDIR
	exx
	ex	de,hl
	ld	hl,R800DATHLP
	ld	bc,R800DATHLP.end-R800DATHLP
	ldir
	ret

; ------------------------------------------------
; R800 optimized data transfer routine, copied to the extra WorkArea
; ------------------------------------------------
R800DATHLP:
	exx
	ld	bc,512
	ldir
	ret
.end:

; ------------------------------------------------
; Z80 optimized data transfer routine, kept in ROM 
; ------------------------------------------------
LDI512:	; Z80 optimized 512 byte transfer
	exx
  REPT 512
	ldi
  ENDM
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
; Input   : none
; Output  : HL': Address of the block transfer routine to be used 
; Modifies: AF, DE', HL'
; ------------------------------------------------
SETLDIRHLPR:
	exx
	; Check for a Z80 or R800
	xor	a		; Clear Cy
	dec	a		; A=#FF
	db	#ED,#F9		; mulub a,a
	jr	c,GTR800LDIR	; Always use LDIR in RAM for the R800

	ld	hl,LDI512
	exx
	ret

; ------------------------------------------------
; Obtain the pointer to the R800 data transfer helper routine
; Input   : IX=Pointer to the WorkArea on SLTWRK
; Output  : HL'=pointer to R800 data transfer helper routine 
; Modifies: Does an exx at the end
; ------------------------------------------------
GTR800LDIR:
	ld	l,(ix+WRKAREA.TRLDIR)
	ld	h,(ix+WRKAREA.TRLDIR+1)
	exx
	ret


; ------------------------------------------------
CLRCTRLSTOP:
; Workaround for a bug on Nextor: Clear the CTRL+STOP
; signal so it wont freeze when booting
; https://github.com/Konamiman/Nextor/issues/1
; ------------------------------------------------
	ld	a,(INTFLG)
	cp	3		; Is CTRL+STOP signaled?
	ret	nz		; no, quit
	xor	a
	ld	(INTFLG),a	; Clear CTRL+STOP otherwise Nextor will freeze
	ret


; ------------------------------------------------
; Debugging routines
; ------------------------------------------------
 IFDEF DEBUG1
PRTCHAR:
	ex	af,af'
	exx
	push	ix
	push	iy
	push	af
	push	bc
	push	de
	push	hl
	exx
	ex	af,af'
	ld	ix,CHPUT
	ld	iy,(EXPTBL-1)
	call	CALSLT
	ex	af,af'
	exx
	pop	hl
	pop	de
	pop	bc
	pop	af
	pop	iy
	pop	ix
	exx
	ex	af,af'
	ret
PRTDOT:
	push	af
	ld	a,'.'
	call	PRTCHAR
	pop	af
	ret
PRTSEMIC:
	push	af
	ld	a,';'
	call	PRTCHAR
	pop	af
	ret
PRTDASH:
	push	af
	ld	a,'-'
	call	PRTCHAR
	pop	af
	ret

PRTHEX:
	push	af
	push	bc
	push	hl
	ld	hl,TEMP3
	call	HexToAscii
	ld	a,'#'
	call	PRTCHAR
	ld	a,(TEMP3)
	call	PRTCHAR
	ld	a,(TEMP3+1)
	call	PRTCHAR
	pop	hl
	pop	bc
	pop	af
	ret


PRTSTRCALL:	; Prints an inline ASCIIZ string
	ex	(sp),hl		; hl=Pointer to inline string
	push	af
	push	ix
	push	iy
	ex	af,af'
	exx
	push	af
	push	bc
	push	de
	push	hl
	exx
	ex	af,af'
	ld	ix,CHPUT
	ld	iy,(EXPTBL-1)
.loop:
	ld	a,(hl)
	inc	hl
	or	a
	jr	z,.end
	call	CALSLT
	jr	.loop

.end:	ex	af,af'
	exx
	pop	hl
	pop	de
	pop	bc
	pop	af
	exx
	ex	af,af'
	pop	iy
	pop	ix
	pop	af
	ex	(sp),hl
	ret
 ENDIF



; ==========================================================================
strTitle:
	db	13,"FBLabs SDXC driver v",27,'J'
	BYTE2STR VER_MAIN
	db	'.'
	BYTE2STR VER_SEC
	db	'.'
	BYTE2STR VER_REV
	db  	13,10 OR #80

;		 |-------------39 chars----------------|
strBootpaused:
	db  	"Paused. Press <i> for the copyright info",13,10 OR #80

strCopyright:
	db	"(c) 2014 Fabio Belavenuto",13,10
	db	"(c) 2017 FRS",13,10
	db	"Licenced under CERN OHL v1.1",13,10
	db	"http://ohwr.org/cernohl",13,10
;	db	"PCB designed by Luciano Sturaro",13,10 OR #80
		; will use the CR+LF+EOS bellow 
strCrLf:
	db	13,10 OR #80
strCartao:
	db	"- Card", " " OR #80
strVazio:
	db	"Empty",13,10 OR #80
strNaoDetectado:
	db	"Failed!",13,10 OR #80
;			 |-------------39 chars----------------|
strMr_mp_desativada:
	db	"- Slot expander & Mem Mapper disabled",13,10 OR #80
 IFDEF HASMEGARAM
strMapper:
	db	"- Slot expander & Mem Mapper enabled",13,10 OR #80
strMegaram:
	db	"- Slot expander & MegaRAM enabled",13,10 OR #80
 ELSE
strDrvMain:
	db	"- Main driver selected",13,10 OR #80
strDrvDev:
	db	"- Development driver selected",13,10 OR #80
 ENDIF
strDskEmu:
	db	"- Floppy disk emulation enabled",13,10 OR #80
strSDV1:
	db 	"SDV1, ","(" OR #80
strSDV2:
	db 	"SDV2, ","(" OR #80

nullTxt:
	db	"<null>"
.end:




;========================================================
; Floppy Emulator

 IFDEF DSKEMU

; MODULE FLOPPYEMU

GETSECT:
	ld	a,(IX+WRKAREA.DSKEMU.DSKDEV)
	or	a			; Is the floppy disk emulation enabled? 
	jr	nz,.doMagic		; Yes, do your magic
	ld	e,(iy+0)		; BC:DE=sector number
	ld	d,(iy+1)
	ld	c,(iy+2)
	ld	b,(iy+3)
	ret				; Just quit

.doMagic:
	; All other DEV/LUNs are blocked for safety reasons
;	ld	a,(IX+WRKAREA.DSKEMU.DSKDEV)
	and	7			; Crop the DEV#
	cp	(IX+WRKAREA.NUMSD)	; Is it the emulated DEV?
	jr	nz,.errorDevLun		; No, quit with error

	push	hl
	call	.chkImgChange		; Check for an image change request

	ld	a,(IX+WRKAREA.DSKEMU.DSKDEV)
	rrca
	rrca
	rrca
	and	#1F			; Crop the Disk number
	ld	de,1440			; Number of sectors on a 720KB disk
	call	Mult12			; hl = sector base offset
	ld	e,(iy+0)		; de = original requested sector
	ld	d,(iy+1)
	add	hl,de			; Point to the desired disk image
	ex	de,hl
	pop	hl

	;Wait for the blitter here
	; FixMe: I've run out of workarea space for both VDP.DW and VDP.DR,
	; so I used only VDP.DW to at least keep the compatibility
	; with the Neos MA-20
	ld	a,(IX+WRKAREA.DSKEMU.VDPDW)
	or	a			; MSX1?
	ret	z			; Yes, quit
	inc	a
	ld	c,a			; c=VDP.DW+1

	ld	a,2
	di
	out	(c),a
	ld	a,#80+15		; Status-register selector
	out	(c),a
;	ld	c,(IX+WRKAREA.DSKEMU.VDPDR)
.waitVDP:
	in	a,(c)
	and	#81
	dec	a
	jr	z,.waitVDP
;	ld	c,(IX+WRKAREA.DSKEMU.VDPDW)
	ld	a,(RG15SA)
	out	(c),a
	ld	a,#80+15		; Status-register selector
	ei
	out	(c),a
	ld	bc,0			; Will always be 0 for up to 20 disks
	xor	a
	ret

.errorDevLun:
	scf
	ret



.chkImgChange:
	; ***FixMe: Some games, like Xak ToG don't like the use of
	; CALSLT here. Since I'm out of free time, I did a QnD
	; implementation with direct I/O access. The CALSLT problem
	; needs further investigation someday

 IFDEF USESNSMAT
	;  ***Xak ToG doesn't seem to like the CALSLTs here
	ex	af,af'
	exx
	push	af,bc,de,hl
	exx
	ex	af,af'
	push	iy,de,ix
	di
	ld	a,6
	ld	ix,SNSMAT
	ld	iy,(EXPTBL-1)
	call	CALSLT
	ld	b,a			; b=KBD_row6
	xor	a	
	ld	ix,SNSMAT
	ld	iy,(EXPTBL-1)
	call	CALSLT
	ld	e,a			; e=KBD_row0
	ld	a,1
	ld	ix,SNSMAT
	ld	iy,(EXPTBL-1)
	call	CALSLT
	ld	d,a			; d=KBD_row1
	ei
	ld	c,b			; c=KBD_row6
 ELSE
	di
	in	a,(#AA)		; Get the current AAh port state
	and	#F0		; Clear the keyboard row
	ld	b,a		; b=KeyClick,CapsLED,CasOut,CasMotor,0000
	or	6		; Select row-6
	out	(#AA),a
	in	a,(#A9)
	ld	c,a		; c=KBD_row6
	ld	a,b		; Select row-0
	out	(#AA),a
	in	a,(#A9)
	ld	e,a		; e=KBD_row0
	ld	a,b
	or	1
	out	(#AA),a
	in	a,(#A9)
	ld	d,a		; d=KBD_row1
	ei
 ENDIF

	ld	b,11
.loop1:
	rr	d
	rr	e		
	jr	nc,.keyPressed
	djnz	.loop1
 IFDEF USESNSMAT
	pop	ix,de,iy
	ex	af,af'
	exx
	pop	hl,de,bc,af
	exx
	ex	af,af'
 ENDIF
	ret

.keyPressed:
	ld	a,10
	sub	b
	jr	nc,.chkShift
	ld	a,10
.chkShift:
	bit	0,c			; Is SHIFT pressed?
	jr	nz,.saveDiskNum		; No, skip
	add	10
.saveDiskNum:
 IFDEF USESNSMAT
	pop	ix
 ENDIF
	add	a
	add	a
	add	a
	ld	b,a
	ld	a,(IX+WRKAREA.DSKEMU.DSKDEV)
	and	#7			; Clear the old Disk number
	or	b			; Mix with the DEV#
	ld	(IX+WRKAREA.DSKEMU.DSKDEV),a
 IFDEF USESNSMAT
	pop	de,iy
	ex	af,af'
	exx
	pop	hl,de,bc,af
	exx
	ex	af,af'
 ENDIF
	ret


;	Multiply 8-bit value with a 16-bit value
;	In: Multiply A with DE
;	Out: HL = result
;
Mult12:
	ld	hl,0
	ld	b,8
Mult12_Loop:
	add	hl,hl
	add	a,a
	jr	nc,Mult12_NoAdd
	add	hl,de
Mult12_NoAdd:
	djnz	Mult12_Loop
	ret

;--------------------------
; Detects if any of the cards have a floppy disk image boot sector
; Output: NZ = Floppy emulation not enabled
;          Z = Floppy emulation enabled
DETIMG:
	xor	a
	ld	(IX+WRKAREA.DSKEMU.DSKDEV),a	; Initialize my Work Area

	ld	a,(IX+WRKAREA.TEMP)	; Get the !KBD_row6
	cpl
	and	3
	ret	z		; No, quit 

	; Select which cardslot will be the single drive A:
	push	bc
	call	CHKBOOTSECT
	pop	bc
	ret	nz		; Not found: quit 

	; Enable the floppy disk emulation
	ld	(IX+WRKAREA.DSKEMU.DSKDEV),d		; Enable the floppy emulation for this DEV
	ret

CHKBOOTSECT:	; Check the cards for an MSX floppy boot sector 
		; Output: Z=found, NZ=Not found
		;         DE=DEV:LUN if found
	ld	a,1		; DEV=1
	ld	c,1		; LUN=1
	call	.chkBootCardSlot
	ld	de,#0101	; DEV=1, LUN=1
	ret	z

	ld	a,2		; DEV=2
	ld	c,1		; LUN=1
	call	.chkBootCardSlot
	ld	de,#0201	; DEV=2, LUN=1
	ret


.chkBootCardSlot:
	; Try to differentiate between floppy boot sectors and mass storage boot sectors
	; Input : A=DEV, C=LUN
	; Output: z=MSX Disk Header found, nz=not found

	ld	b,1			; Sectors=1
	ld	de,WRKTEMP.SECTNUM
	ld	hl,0
	ld	(WRKTEMP.SECTNUM),hl
	ld	(WRKTEMP.SECTNUM+2),hl
	ld	hl,WRKTEMP.BUFFER
	or	a			; Clear Cy

	call	DEV_RW
	or	a			; Any error? (including no disk)
	ret	nz			; Yes, quit with NZ (no floppy image found)

	; First we check the boot signature 
	ld	a,(WRKTEMP.BUFFER+#000)		; Get this boot sector signature 
	cp	#E9			; Starts with E9h?
	ret	z			; Assume that this is a custom disk image
	cp	#EB			; Starts with EBh?
	ret	nz			; This is not a bootable disk

	; FAT16 signature?
	ld	hl,.FAT16sig
	ld	de,WRKTEMP.BUFFER+#036	; FAT16 signature position 
	ld	bc,#0500		; 5 bytes long
.loop1:	ld	a,(de)
	cpi
	jr	nz,.chkFAT32		; Not FAT16, skip
	inc	de
	djnz	.loop1
	jr	.notDsk			; Found FAT16: this isn't a floppy	

.chkFAT32:
	; FAT32 signature?
	ld	hl,.FAT32sig
	ld	de,WRKTEMP.BUFFER+#052	; FAT32 signature position 
	ld	bc,#0500		; 5 bytes long
.loop2:	ld	a,(de)
	cpi
	jr	nz,.chkPCbootsig	; Not FAT32, skip
	inc	de
	djnz	.loop2
	jr	.notDsk			; Found FAT32: this isn't a floppy	

.chkPCbootsig:
	; Is there a PC boot signature at 0x1FE?
	ld	hl,(WRKTEMP.BUFFER+#01FE)	; Get this boot sector signature 
	ld	de, #AA55		; PC boot signature
	or	a
	sbc	hl,de			; Is there a PC boot signature here?
	jr	z,.notDsk		; Yes, then it's not an MSX floppy

	; Then we check for a custom boot sector
	ld	a,(WRKTEMP.BUFFER+#002)		; Get this boot sector signature 
	cp	#90			; Is the offset +2 = FEh?
	jr	nz,.isDsk		; No, then assume this is a custom DiskBIOS1 boot

	; Are there non-ASCII chars in the OEM name field?
;	ld	hl,WRKTEMP.BUFFER+#003	; OEM field position 
	;***TODO

	; Then we check for a standard boot sector structure 
	ld	a,(WRKTEMP.BUFFER+#01E)	; Get DiskBIOS1 init instruction
	ld	hl,.MSXdsk1bootsi
	ld	bc,.MSXdsk1bootse-.MSXdsk1bootsi
	cpir				; Any known diskBIOS1 init instruction?
	ret	z			; Yes, return

	ld	a,(WRKTEMP.BUFFER+#015)	; Get the media descriptor
	ld	hl,.MSXmediadesc
	ld	bc,4
	cpir				; Check if is one of the MSX floppy descriptors
	ret	nz			; No, quit with NZ

	ld	de,(WRKTEMP.BUFFER+#013)	; Get the number of sectors
	ld	a,e
	or	d			; 0 sectors?
	jr	z,.notDsk		; Then it's not a dsk image, quit
	ld	hl,82*9*2+1		; Max 82 tracks, 9 sects/trk, 2 sides
	sbc	hl,de			; More than the maximum # of sectors?
	jr	c,.notDsk		; Then it's not a dsk image, quit

	; If we got here, this is a floppy DSK image,
	xor	a
	ret			; quit with Z set


.isDsk:		; This is an MSX1 DSK image (sometimes custom)
	xor	a
	ret

.notDsk:	; This is not a DSK image
	ld	a,1
	or	a
	ret	; Quit with NZ


.MSXmediadesc:	db	#F9,#F8,#FB,#FA		; BPB: Media descriptors used by the MSX
.MSXdsk1bootsi:	; Known instructions used at offset 01Eh to boot diskBIOS1 disks that aren't used for mass storage
		db	#D0,#F3,#C3,#38,#DC,#DA,#21,#11
.MSXdsk1bootse:	; End of the known instructions
.FAT16sig:	db	"FAT16   "
.FAT32sig:	db	"FAT32   "

; ENDMOD


 ENDIF ; IFDEF DSKEMU

;=======================
; Variables
;=======================
	.phase	#C000
WRKTEMP.BUFFER:		ds 512 		; Buffer for the IDENTIFY info
WRKTEMP.SECTNUM:	ds 4		; Pointer to the text "Master:" or "Slave:"
	.dephase


;-----------------------------------------------------------------------------
;
; End of the driver code

DRV_END:

;	ds	3ED0h-(DRV_END-DRV_START), #FF
	ds	#7B00 - $, #FF

end
