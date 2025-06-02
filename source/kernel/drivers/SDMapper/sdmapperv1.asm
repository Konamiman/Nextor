	.CPU Z80
	TITLE Nextor driver for SD Mapper/Megaram v1.0.7
	.relab

; Projeto MSX SD Mapper

; Copyright (c) 2014 Fabio Belavenuto
; Enhanced by FRS

; This documentation describes Open Hardware and is licensed under the CERN OHL v. 1.1.
; You may redistribute and modify this documentation under the terms of the
; CERN OHL v.1.1. (http://ohwr.org/cernohl). This documentation is distributed
; WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF MERCHANTABILITY,
; SATISFACTORY QUALITY AND FITNESS FOR A PARTICULAR PURPOSE.
; Please see the CERN OHL v.1.1 for applicable conditions

; Technical info:
; 4000h~47FFh	: SPI data transfer window (read/write)
; 4800h		: Interface status and card select register (read/write)
;	<read>
;	b0	: 0=SD card present on slot-0
;	b1	: 0=SD card present on slot-1
;	b2	: 0=Write protecton enabled for SD card slot-0
;	b3	: 0=Write protecton enabled for SD card slot-1
;	b4	: SW0 status. 0=RAM enabled, 1=RAM disabled
;	b5	: SW1 status. 0=RAM mode: MegaRAM, 1=RAM mode: Memory Mapper
;	b6	: Reserved for future use. Must be masked out from readings.
;	b7	: 1=SPI transfer busy. 0=No ongoing SPI transfer
;	<write>
;	b0	: SD card slot-0 chip-select (0=selected)
;	b1	: SD card slot-1 chip-select (0=selected)

; 6001h		: Mode configuration register (write only)
;	b0	: 0=ROM mode, SPI interface disabled, 1=SPI interface enabled

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

DRV_HOTPLUG	equ	1

DEBUG	equ	0	;Set to 1 for debugging, 0 to normal operation

;Driver version

VER_MAIN	equ	1
VER_SEC		equ	0
VER_REV		equ	7

;-----------------------------------------------------------------------------
; SPI addresses. Check the Technical info above for the bit contents

CHAVEIASPI	equ #6001 ; Mode config register
PORTCFG		equ #4800	; Interface status and card select register
PORTSPI		equ #4000	; SPI data port

; Comandos SPI:
CMD0	equ 0  OR #40
CMD1	equ 1  OR #40
CMD8	equ 8  OR #40
CMD9	equ 9  OR #40
CMD10	equ 10 OR #40
CMD12	equ 12 OR #40
CMD16	equ 16 OR #40
CMD17	equ 17 OR #40
CMD18	equ 18 OR #40
CMD24	equ 24 OR #40
CMD25	equ 25 OR #40
CMD55	equ 55 OR #40
CMD58	equ 58 OR #40
ACMD23	equ 23 OR #40
ACMD41	equ 41 OR #40

; Work area variables
BCSD		equ 0				; Card Specific Data
BCID1		equ BCSD+16			; Card-ID of card1
BCID2		equ BCID1+16		; Card-ID of card2
NUMSD		equ BCID2+16		; Currently selected card: 1 or 2
CARDFLAGS	equ NUMSD+1			; Flags that indicate card-change or card error
NUMBLOCKS	equ CARDFLAGS+1		; Number of blocks in multi-block operations
BLOCKS1		equ NUMBLOCKS+1		; 3 bytes. Size of card1, in blocks.
BLOCKS2		equ BLOCKS1+3		; 3 bytes. Size of card2, in blocks.
TEMP		equ BLOCKS2+3		; Temporary data
TRLDIR		equ TEMP+1			; R800 data transfer helper
WORKAREA_LEN	equ TRLDIR+8

;-----------------------------------------------------------------------------
;
; Standard BIOS and work area entries
INITXT	equ #006C		; Inicializa SCREEN0
CHSNS	equ #009C		; Sense keyboard buffer for character
CHGET	equ #009F		; Get character from keyboard buffer
CHPUT	equ #00A2		; A=char
CLS		equ #00C3		; Chamar com A=0
ERAFNK	equ #00CC		; Erase function key display
SNSMAT	equ #0141		; Read row of keyboard matrix
KILBUF	equ #0156		; Clear keyboard buffer
EXTROM	equ #015F

; subROM functions
SDFSCR	equ #0185
REDCLK	equ #01F5


; System variables
MSXVER	equ #002D
LINL40	equ #F3AE		; Width
LINLEN	equ #F3B0
INTFLG	equ #FC9B
SCRMOD	equ #FCAF


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

CODE_ADD:	equ	0F1D0h


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

	db 1+(2*DRV_HOTPLUG)

;-----------------------------------------------------------------------------
;
; Reserved byte
;
	db	0

;-----------------------------------------------------------------------------
;
; Driver name
;

DRV_NAME:
	db	"SDMapper Driver"
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

	ds	15

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


; Skips the interface r/w registers
	ds	#4800-$, #FF


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
	jr	nz,.call2
; 1st call:
	ld	hl,TRLDIR ; size of work area are needed for Z80
	ld	a,(MSXVER)
	cp	3		; MSX Turbo-R?
	ccf
	ret	nc		; No, return with Cy off
	ld	hl, WORKAREA_LEN	; size of work area are needed for TR
	or	a		; Clear Cy
	ret


.call2:
; 2nd call:
	call	MYSETSCR		; Set the screen mode
	call	pegaWorkArea		; HL=IY=Work area pointer

	ld	de,strTitle		; prints the title
	call	printString

	call	SDMAPPERINIT		; SD-Mapper exclusive init

.sdhcinit:	; FBLabs SDHC Interface initialization
	xor	a			; zera flags do cartao
	ld	(iy+CARDFLAGS), a

	ld	a, 1			; detectar cartao 1
	call	.detecta
	ld	a, 2			; detectar cartao 2
	call	.detecta
	ld	bc, 0
	ld	e, 5
	call	modoROM

	call	INSTR800HLP		; Install R800 data copy on workarea

	call	INICHKSTOP		; Check if the STOP key was pressed

	ld	de, strCrLf
	jp	printString

.detecta:
	ld	(iy+NUMSD), a		; processo de deteccao dos cartoes
	ld	de, strCartao
	call	printString
	ld	a, (iy+NUMSD)
	add	'0'
	call	CHPUT
	ld	a, ':'
	call	CHPUT
	ld	a, ' '
	call	CHPUT
	ld	c, (iy+NUMSD)
	ld	a, (PORTCFG)		; testar se cartao esta inserido
	and	c			; C contem 1 se cartao 1, 2 se cartao 2
	jr	z,.naoVazio
	ld	de, strVazio		; nao tem cartao no slot
	call	printString
	jp	.marcaErro
.naoVazio:
	call	detectaCartao		; tem cartao no slot, inicializar e detectar
	jr	nc,.detectou
	call	desabilitaSDs
	ld	de, strNaoIdentificado
	call	printString
.marcaErro:
	jp	marcaErroCartao		; slot vazio ou erro de deteccao, marcar nas flags
.detectou:
	call	calculaCIDoffset	; calculamos em IX a posicao correta do offset CID dependendo do cartao atual
	ld	a,(ix+15)	; pegar byte SDV1 ou SDV2
	ld	de, strSDV1	; e imprimir
	or	a
	jr	z,.pula1
	ld	de, strSDV2
.pula1:
	call	printString
	ld	a,'('
	call	CHPUT
	ld	a,(ix)		; pegar byte do fabricante
	call	printDecToAscii	; Imprimir Manufacturer ID
	ld	a,')'
	call	CHPUT
	ld	a,' '
	call	CHPUT
	ld	a,(ix)		; pegar byte do fabricante
	call	pegaFabricante	; achar nome do fabricante
	ex	de,hl
	call	printString	; e imprimir
	ld	de,strCrLf
	call	printString
	ret


SDMAPPERINIT:		; This block is exclusive for the SD-Mapper
	ld	de, strMr_mp_desativada
	call	modoSPI
	ld	a, (PORTCFG)		; testar se mapper/megaram esta ativa
	and	#10
	jr	z,.print		; desativada, pula
	ld	de, strMapper
	ld	a, (PORTCFG)		; ativa, testar se eh mapper ou megaram
	and	#20
	jp	nz,printString
	ld	de, strMegaram		; Megaram ativa
.print:
	jp	printString

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
	cp	a
	cp	3		; somente 2 dispositivos
	jr	nc,.saicomerroidl
	dec	c		; somente 1 logical unit
	jr	nz,.saicomerroidl
	push	hl
	call	testaCartao	; verificar se cartao esta OK
	pop	hl
	jr	nc,.ok
	pop	af
	ld	a, ENRDY	; Not ready
;	ld	a, EDISK	; General unknown disk error
	ld	b, 0
	ret
.saicomerroidl:
	pop	af
	ld	a, EIDEVL	; error: Invalid device or LUN
	ld	b, 0
	ret
.ok:
	ld	(iy+NUMBLOCKS), b	; save the number of blocks to transfer
	exx
	call	calculaCIDoffset	; ix=CID offset
	ld	a,(ix+15)	; verificar se eh SDV1 ou SDV2
	ld	ixl,a		; ixl=SDcard version
	pop	af		; a=Device number, f=read/write flag
	exx			; hl=Source/dest Address, de=Pointer to sect#
	ld	ixh, b 		; ixh=Number of blocks to transfer
	jr	c,escrita	; Skip if it's a write operation
leitura:
	ld	a, (de)		; 1. n. bloco
	push	af
	inc	de
	ld	a, (de)		; 2. n. bloco
	push	af
	inc	de
	ld	a, (de)		; 3. n. bloco
	ld	c, a
	inc	de
	ld	a, (de)		; 4. n. bloco
	inc	de
	ld	b, a
	pop	af
	ld	d, a
	pop	af		; HL = ponteiro destino
	ld	e, a		; BC DE = 32 bits numero do bloco
	call	modoSPI
	call	LerBloco	; chamar rotina de leitura de dados
	call	modoROM
	jr	nc,.ok
	call	marcaErroCartao		; ocorreu erro na leitura, marcar erro
	ld	a,(iy+NUMBLOCKS) ; Get the number of requested blocks
	sub	ixh		; subtract the number of remaining blocks
	ld	b,a		; b=number of blocks read
;	ld	a, ENRDY	; Not ready
	ld	a, EDISK	; General unknown disk error
.ok:
	xor	a		; tudo OK, informar ao Nextor
	ret

escrita:
	; Test if the card is write protected
	ld	c,(iy+NUMSD)	; cartao atual (1 ou 2)
	sla	c		; desloca para apontar para bits 2 ou 3 para cartoes 1 ou 2 respectivamente
	sla	c
	call	modoSPI
	ld	a,(PORTCFG)	; testar se cartao esta protegido
	call	modoROM
	and	c
	jr	z,.ok
	ld	a, EWPROT	; disco protegido
	ld	b,0		; 0 blocks were written
	ret
.ok:
	ld	a, (de)		; 1. n. bloco
	push	af
	inc	de
	ld	a, (de)		; 2. n. bloco
	push	af
	inc	de
	ld	a, (de)		; 3. n. bloco
	inc	de
	ld	c, a
	ld	a, (de)		; 4. n. bloco
	inc	de
	ld	b, a
	pop	af
	ld	d, a
	pop	af		; HL = ponteiro destino
	ld	e, a		; BC DE = 32 bits numero do bloco
	call	modoSPI
	call	GravarBloco	; chamar rotina de gravacao de dados
	call	modoROM
	jr	nc,.ok2
	call	marcaErroCartao		; ocorreu erro, marcar nas flags
	ld	a,(iy+NUMBLOCKS) ; Get the number of requested blocks
	sub	ixh		; subtract the number of remaining blocks
	ld	b,a		; b=number of blocks written
	ld	a,EWRERR	; Write error
	ret
.ok2:
	xor	a			; gravacao sem erros!
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
	inc	b
	cp	a
	cp	3		; somente 2 dispositivos
	jr	nc,.saicomerro
	push	hl
	call	testaCartao	; verificar se cartao esta OK
	pop	hl
	jr	nc,.ok		; nao houve erro
.saicomerro:
	ld	a,2		; informar erro
	ret

.ok:
	djnz	.naoBasic

; Basic information:
	ld	(hl), 1		; 1 logical unit somente
	inc	hl
	ld	(hl), 0		; reservado, deve ser 0
	ret			; retorna com A=0 (OK)

.naoBasic:
	push	hl
	call	calculaCIDoffset	; calculamos em IX a posicao correta do offset CID dependendo do cartao atual
	pop	hl

	djnz	.naoManuf
; Manufacturer Name:
	push	hl		; salva ponteiro do buffer
	ld	b, 64		; preenche buffer com espaco
	ld	a, ' '
.loop1:
	ld	(hl), a
	inc	hl
	djnz	.loop1
	pop	de		; recuperamos ponteiro do buffer em DE
	ld	a, '('		; colocamos (xx) xxx no buffer
	ld	(de), a
	inc	de
	ld	a, (ix)		; byte do fabricante
	call	DecToAscii
	ld	a, ')'
	ld	(de), a
	inc	de
	ld	a, ' '
	ld	(de), a
	inc	de
	ld	a, (ix)		; byte do fabricante
	call	pegaFabricante	; pegar nome do fabricante em HL
	ldir			; e colocar no buffer
	ret

.naoManuf:
	djnz	.naoProduct
; Product Name:
	push	hl		; guarda HL que aponta para buffer do Nextor
	push	ix
	pop	hl		; joga IX para HL
	ld	d, 0
	ld	e, 3		; adiciona offset do productname em HL
	add	hl, de
	pop	de		; recupera buffer do Nextor em DE
	ld	bc, 5		; 5 caracteres
	ldir			; copia nome do produto
	ex	de,hl		; troca DE com HL, agora HL aponta para Buffer do nextor atualizado
	ld	b, 59		; Coloca espaco no restante do buffer
	ld	a, ' '
.loop2:
	ld	(hl), a
	inc	hl
	djnz	.loop2
	xor	a		; informar sem erros
	ret

.naoProduct:
; Serial Number:
	ld	(hl),'0'	; Coloca prefixo "0x"
	inc	hl
	ld	(hl), 'x'
	inc	hl
	push	hl		; guarda HL que aponta para buffer do Nextor
	push	ix
	pop	hl		; joga IX para HL
	ld	d, 0
	ld	e, 9		; adiciona offset do productname em HL
	add	hl, de
	pop	de		; recupera buffer do nextor em DE
	ld	b, 4		; 4 bytes do serial
.loop3:
	ld	a, (hl)
	call	HexToAscii	; converter HEXA para ASCII
	inc	hl
	djnz	.loop3
	ld	b, 54		; Coloca espaco no restante
	ld	a, ' '
.loop4:
	ld	(de), a
	inc	de
	djnz	.loop4
	xor	a		; informar sem erros
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
	cp	a
	cp	3		; 2 dispositivos somente
	jr	nc,.saicomerro
	dec	b		; 1 logical unit somente
	jr	nz,.saicomerro
	push	af
	call	pegaWorkArea	; HL=IY=Work area pointer
	pop	af
	ld	(iy+NUMSD),a	; salva numero do device atual (1 ou 2)
	ld	c,a		; c=device number
	call	modoSPI
	ld	a, (PORTCFG)	; testar se cartao esta inserido
	call	modoROM
	and	c		; C contem 1 se cartao 1, 2 se cartao 2
	jr	nz,.cartaoAusente	; se slot do cartao estiver vazio, marcamos o erro nas flags
	;***FixMe: Disk change detection doesn't work
	; Will have to check both the hardware disk-change and the software
	; disk-change reported by LUN_NFO
	call	modoSPI
	call	detectaCartao	; erro na deteccao do cartao, tentar re-detectar
	call	modoROM
	jr	c,.cartaoComErro	; nao conseguimos detectar, sai com erro

.semMudanca:
	ld	a, 1		; informa ao Nextor que cartao esta OK e nao mudou
	ret
.comMudanca:
	ld	a, 2		; informa ao Nextor que cartao esta OK e mudou
	ret
.cartaoComErro:
.cartaoAusente:
	call	marcaErroCartao	; marcar erro do cartao nas flags
.saicomerro:
	ld	a, 0		; informa erro
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
	cp	a
	cp	3		; somente 2 dispositivo
	jr	nc,.saicomerro
	dec	b		; somente 1 logical unit
	jr	nz,.saicomerro
	push	hl
	call	testaCartao
	pop	hl
	jr	nc,.ok		; nao tem erro com o cartao
.saicomerro:
	ld	a, 1		; informar erro
	ret
.ok:
	exx
	; ***FixMe: Will have to check disk-change, and daisy chain this as a
	; software flag so DEV_STATUS can also be notified
	call	modoSPI
	call	detectaCartao	; erro na deteccao do cartao, tentar re-detectar
	call	modoROM
	jr	c,.saicomerro
	call	calculaBLOCOSoffset	; calcular em IX e HL o offset correto do buffer que armazena total de blocos
	exx			; do cartao dependendo do cartao atual solicitado
	xor	a
	ld	(hl),a		; Informar que o dispositivo eh do tipo block device
	inc	hl
	ld	(hl),a		; block size: 512 bytes (0200h)
	inc	hl
	ld	(hl),2
	inc	hl
	ld	a, (ix)		; copia numero de blocos total
	ld	(hl), a
	inc	hl
	ld	a, (ix+1)
	ld	(hl), a
	inc	hl
	ld	a, (ix+2)
	ld	(hl), a
	inc	hl
	ld	(hl),0		; The highest byte must be set to 0, as SDcards
				; have 24bit total block numbers, and Nextor
				; requires 32bits.
	inc	hl
	ld	(hl),1		; flags: dispositivo R/W removivel
	inc	hl
	xor	a		; CHS = 0
	ld	(hl), a
	inc	hl
	ld	(hl), a
	inc	hl
	ld	(hl), a
;	xor	a		; informar que dados foram preenchidos
	ret

;=====
;=====  END of DEVICE-BASED specific routines
;=====

;------------------------------------------------
; Rotinas auxiliares
;------------------------------------------------

;------------------------------------------------
; Pedir ao Nextor o ponteiro de dados de trabalho
; na RAM e colocar em HL e IY
; Destroi HL e IY
;------------------------------------------------
pegaWorkArea:
	push	af
	xor	a		; Pegar endereco da area de trabalho
	ex	af,af'
	xor	a
	ld	ix, GWORK
	call	modoROM
	call	CALBNK
	ld	l,(ix)		; em HL tem o ponteiro da nossa area da RAM
	ld	h,(ix+1)
	push	hl
	pop	iy		; em IY temos o mesmo ponteiro
	pop	af
	ret

;------------------------------------------------
; Testa se cartao esta inserido e/ou houve erro
; na ultima vez que foi acessado. Carry indica
; erro
; Destroi AF, HL, IX, C
;------------------------------------------------
testaCartao:
	push	af
	call	pegaWorkArea	; HL=IY=Work area pointer
	pop	af
	ld	(iy+NUMSD), a	; salva numero do device atual (1 ou 2)
	ld	c, a
	call	modoSPI
	ld	a, (PORTCFG)	; testar se cartao esta inserido
	call	modoROM
	and	c		; C contem 1 se cartao 1, 2 se cartao 2
	jr	nz,.saicomerro
	ld	a, (iy+NUMSD)		; conseguimos detectar, tira erro nas flags
	cpl			; inverte bits para fazer o AND
	ld	c, a
	ld	a, (iy+CARDFLAGS)
	and	c			; limpa bit
	ld	(iy+CARDFLAGS), a
	ret
.saicomerro:
	ld	a, (iy+CARDFLAGS)	; marca bit de erro nas flags
	or	c
	ld	(iy+CARDFLAGS), a
	scf
	ret

;------------------------------------------------
; Marcar bit de erro nas flags
; Destroi AF, C
;------------------------------------------------
marcaErroCartao:
	ld	c, (iy+NUMSD)		; cartao atual (1 ou 2)
	ld	a, (iy+CARDFLAGS)	; marcar erro
	or	c
	ld	(iy+CARDFLAGS), a
	ret

;------------------------------------------------
; Calcula offset do buffer na RAM em HL e IX para
; os dados do CID dependendo do tipo do cartao atual
; Destroi AF, DE, HL e IX
;------------------------------------------------
calculaCIDoffset:
	push	iy		; copiamos IY para HL
	pop	hl
	ld	d, 0
	ld	e, BCID1	; DE aponta para buffer BCID1
	ld	a, (iy+NUMSD)	; vamos fazer IX apontar para o buffer correto
	dec	a		; dependendo do cartao: BCID1 ou BCID2
	jr	z,.c1
	ld	e, BCID2	; DE aponta para buffer BCID2
.c1:
	add	hl, de		; HL aponta para buffer correto
	push	hl
	pop	ix		; vamos colocar HL em IX
	ret

;------------------------------------------------
; Calcula offset do buffer na RAM para os dados
; do total de blocos dependendo do cartao atual
; Offset fica em HL e IX
; Destroi AF, DE, HL e IX
;------------------------------------------------
calculaBLOCOSoffset:
	push	iy		; copiamos IY para HL
	pop	hl
	ld	d, 0
	ld	e, BLOCKS1	; DE aponta para buffer BLOCKS1
	ld	a, (iy+NUMSD)	; Vamos fazer IX apontar para o buffer correto
	dec	a		; dependendo do cartao: BLOCKS1 ou BLOCKS2
	jr	z,.c1
	ld	e, BLOCKS2	; DE aponta para buffer BLOCKS2
.c1:
	add	hl, de		; HL aponta para buffer correto
	push	hl
	pop	ix		; Vamos colocar HL em IX
	ret


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
;------------------------------------------------
detectaCartao:
	call	iniciaSD	; manda pulsos de clock e comandos iniciais
	ret	c			; retorna se erro
	call	testaSDCV2	; tenta inicializar um cartao SDV2
	ret	c
	push	iy		; colocar em HL buffer da RAM de trabalho
	pop	hl		; CSD esta no offset 0, nao precisamos somar
	ld	a, CMD9		; ler CSD
	call	lerBlocoCxD
	ret	c
	call	calculaCIDoffset	; calculamos em IX e HL a posicao correta do offset CID dependendo do cartao atual
	ld	a, CMD10	; ler CID
	call	lerBlocoCxD
	ret	c
	ld	a, CMD58	; ler OCR
	ld	de, 0
	call	SD_SEND_CMD_2_ARGS_GET_R3	; enviar comando e receber resposta tipo R3
	ret	c
	ld	a, b		; testa bit CCS do OCR que informa se cartao eh SDV1 ou SDV2
	and	#40
	ld	(ix+15), a	; salva informacao da versao do SD (V1 ou V2) no byte 15 do CID
	call	z,mudarTamanhoBlocoPara512	; se bit CCS do OCR for 1, eh cartao SDV2 (Block address - SDHC ou SDXD)
	ret	c		; e nao precisamos mudar tamanho do bloco para 512
	call	desabilitaSDs
				; agora vamos calcular o total de blocos dependendo dos dados do CSD
	call	calculaBLOCOSoffset	; calcular em IX e HL o offset correto do buffer que armazena total de blocos
	push	iy		; copiamos IY para HL
	pop	hl
	ld	d, 0
	ld	e, BCSD+5
	add	hl, de		; HL aponta para buffer BCSD+5
	ld	a, (iy+BCSD)
	and	#C0		; testa versao do registro CSD
	jr	z,.calculaCSD1
	cp	#40
	jr	z,.calculaCSD2
	scf			; versao do registro CSD nao reconhecida, informa erro na deteccao
	ret

; -----------------------------------
; Registro CSD versao 1, calcular da
; maneira correta para a versao 1
; -----------------------------------
.calculaCSD1:
	ld	a, (hl)
	and	#0F		; isola READ_BL_LEN
	push	af
	inc	hl
	ld	a, (hl)		; 2 primeiros bits de C_SIZE
	and	3
	ld	d, a
	inc	hl
	ld	e, (hl)		; 8 bits de C_SIZE (DE contem os primeiros 10 bits de C_SIZE)
	inc	hl
	ld	a, (hl)
	and	#C0		; 2 ultimos bits de C_SIZE
	add	a, a		; rotaciona a esquerda
	rl	e		; rotaciona para DE
	rl	d
	add	a, a		; mais uma rotacao
	rl	e		; rotaciona para DE
	rl	d
	inc	de		; agora DE contem todos os 12 bits de C_SIZE, incrementa 1
	inc	hl
	ld	a, (hl)		; proximo byte
	and	3		; 2 bits de C_SIZE_MUL
	ld	b, a		; B contem os 2 bits de C_SIZE_MUL
	inc	hl
	ld	a, (hl)		; proximo byte
	and	#80		; 1 bit de C_SIZE_MUL
	add	a, a		; rotaciona para esquerda jogando no carry
	rl	b		; rotaciona para B
	inc	b		; agora B contem os 3 bits de C_SIZE_MUL
	inc	b		; faz B = C_SIZE_MUL + 2
	pop	af		; volta em A o READ_BL_LEN
	add	a, b		; A = READ_BL_LEN + (C_SIZE_MUL+2)
	ld	bc, 0
	call	.eleva2
	ld	e, d		; aqui temos 32 bits (BC DE) com o tamanho do cartao
	ld	d, c		; ignoramos os 8 ultimos bits em E, fazemos BC DE => 0B CD (divide por 256)
	ld	c, b
	ld	b, 0
	srl	c		; rotacionamos a direita o C, carry = LSB (divide por 2)
	rr	d		; rotacionamos D e E
	rr	e		; no final BC DE contem tamanho do cartao / 512 = numero de blocos
.salvaBlocos:
	ld	(ix+2), c	; colocar no buffer BLOCOS correto a quantidade de blocos
	ld	(ix+1), d	; que o cartao (1 ou 2) tem
	ld	(ix), e
	xor	a		; limpa carry
	ret

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
; Registro CSD versao 2, calcular da
; maneira correta para a versao 2
; -----------------------------------
.calculaCSD2:
	inc	hl		; HL ja aponta para BCSD+5, fazer HL apontar para BCSD+7
	inc	hl
	ld	a, (hl)
	and	#3F
	ld	c, a
	inc	hl
	ld	d, (hl)
	inc	hl
	ld	e, (hl)
	call	.inc32		; soma 1
	call	.desloca32	; multiplica por 512
	call	.rotaciona24	; multiplica por 2
	jp		.salvaBlocos

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

; ------------------------------------------------
; Setar o tamanho do bloco para 512 se o cartao
; for SDV1
; ------------------------------------------------
mudarTamanhoBlocoPara512:
	ld	a, CMD16
	ld	bc, 0
	ld	de, 512
	jp	SD_SEND_CMD_GET_ERROR

; ------------------------------------------------
; Tenta inicializar um cartao SDV2, se houver erro
; o cartao deve ser SDV1
; ------------------------------------------------
testaSDCV2:
	ld	a, CMD8
	ld	de, #1AA
	call	SD_SEND_CMD_2_ARGS_GET_R3
	ld	hl, SD_SEND_CMD1	; HL aponta para rotina correta
	jr	c,.pula		; cartao recusou CMD8, enviar comando CMD1
	ld	hl, SD_SEND_ACMD41	; cartao aceitou CMD8, enviar comando ACMD41
.pula:
	ld	bc, 20		; B = 0, C = 20: 5120 tentativas
.loop:
	push	bc
	call	.jumpHL		; chamar rotina correta em HL
	pop	bc
	ret	nc
	djnz	.loop
	dec	c
	jr	nz,.loop
	scf
	ret
.jumpHL:
	jp	(hl)		; chamar rotina correta em HL

; ------------------------------------------------
; Ler registro CID ou CSD, o comando vem em A
; ------------------------------------------------
lerBlocoCxD:
	call	SD_SEND_CMD_NO_ARGS
	ret	c
	call	WAIT_RESP_FE
	ret	c
	ex	de,hl

	ld	hl, PORTSPI
 rept 16
	ldi		; LDI x16
 endm
	ld	a, (hl)
	ld	a, (hl)		; byte de resposta
	or	a
	ex	de,hl
	jr	desabilitaSDs

; ------------------------------------------------
; Algoritmo para inicializar um cartao SD
; Destroi AF, B, DE
; ------------------------------------------------
iniciaSD:
	call	desabilitaSDs

	ld	b, 10		; enviar 80 pulsos de clock com cartao desabilitado
enviaClocksInicio:
	ld	a, #FF		; manter MOSI em 1
	ld	(PORTSPI), a
	djnz	enviaClocksInicio
	call	setaSDAtual	; ativar cartao atual
	ld	b, 8		; 8 tentativas para CMD0
SD_SEND_CMD0:
	ld	a, CMD0		; primeiro comando: CMD0
	ld	de, 0
	push	bc
	call	SD_SEND_CMD_2_ARGS_TEST_BUSY
	pop	bc
	ret	nc			; retorna se cartao respondeu ao CMD0
	djnz	SD_SEND_CMD0
	scf			; cartao nao respondeu ao CMD0, informar erro
	; fall throw

; ------------------------------------------------
; Desabilitar (de-selecionar) todos os cartoes
; Nao destroi registradores
; ------------------------------------------------
desabilitaSDs:
	push	af
	ld	a, #FF		; todos os /CS em 1
	ld	(PORTCFG), a
	pop	af
	ret

; ------------------------------------------------
; Enviar comando ACMD41
; ------------------------------------------------
SD_SEND_ACMD41:
	ld	a, CMD55
	call	SD_SEND_CMD_NO_ARGS
	ld	a, ACMD41
	ld	bc, #4000
	ld	d, c
	ld	e, c
	jr		SD_SEND_CMD_GET_ERROR

; ------------------------------------------------
; Enviar CMD1 para cartao. Carry indica erro
; Destroi AF, BC, DE
; ------------------------------------------------
SD_SEND_CMD1:
	ld	a, CMD1
SD_SEND_CMD_NO_ARGS:
	ld	bc, 0
	ld	d, b
	ld	e, c
SD_SEND_CMD_GET_ERROR:
	call	SD_SEND_CMD
	or	a
	ret	z			; se A=0 nao houve erro, retornar
	; fall throw

; ------------------------------------------------
; Informar erro
; Nao destroi registradores
; ------------------------------------------------
setaErro:
	scf
	jr		desabilitaSDs

; ------------------------------------------------
; Enviar comando em A com 2 bytes de parametros
; em DE e testar retorno BUSY
; Retorna em A a resposta do cartao
; Destroi AF, BC
; ------------------------------------------------
SD_SEND_CMD_2_ARGS_TEST_BUSY:
	ld	bc, 0
	call	SD_SEND_CMD
	ld	b, a
	and	#FE		; testar bit 0 (flag BUSY)
	ld	a, b
	jr	nz,setaErro	; BUSY em 1, informar erro
	ret			; sem erros

; ------------------------------------------------
; Enviar comando em A com 2 bytes de parametros
; em DE e ler resposta do tipo R3 em BC DE
; Retorna em A a resposta do cartao
; Destroi AF, BC, DE, HL
; ------------------------------------------------
SD_SEND_CMD_2_ARGS_GET_R3:
	call	SD_SEND_CMD_2_ARGS_TEST_BUSY
	ret	c
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
; em BC DE e enviar CRC correto se for CMD0 ou
; CMD8 e aguardar processamento do cartao
; Output  : A=0 if there was no error
; Modifies:  AF, B, AF'
; ------------------------------------------------
SD_SEND_CMD:
	ex	af,af'
	call	setaSDAtual
	ex	af,af'
	ld	(PORTSPI), a
	ex	af,af'
	ld	a, b
	ld	(PORTSPI), a
	ld	a, c
	ld	(PORTSPI), a
	ld	a, d
	ld	(PORTSPI), a
	ld	a, e
	ld	(PORTSPI), a
	ex	af,af'
	cp	CMD0
	ld	b, #95		; CRC para CMD0
	jr	z,enviaCRC
	cp	CMD8
	ld	b, #87		; CRC para CMD8
	jr	z,enviaCRC
	ld	b, #FF		; CRC dummy
enviaCRC:
	ld	a, b
	ld	(PORTSPI), a
;	jr	WAIT_RESP_NO_FF

; ------------------------------------------------
; Esperar que resposta do cartao seja diferente
; de #FF
; Destroi AF, BC
; ------------------------------------------------
WAIT_RESP_NO_FF:
	ld	bc, 1		; 256 tentativas
.loop:
	ld	a, (PORTSPI)
	cp	#FF		; testa #FF
	ret	nz		; sai se nao for #FF
	djnz	.loop
	dec	c
	jr	nz,.loop
	ret

; ------------------------------------------------
; Esperar que resposta do cartao seja #FE
; Destroi AF, B
; ------------------------------------------------
WAIT_RESP_FE:
	ld	b, 10		; 10 tentativas
.loop:
	push	bc
	call	WAIT_RESP_NO_FF	; esperar resposta diferente de #FF
	pop	bc
	cp	#FE		; resposta é #FE ?
	ret	z		; sim, retornamos com carry=0
	djnz	.loop
	scf			; erro, carry=1
	ret

; ------------------------------------------------
; Esperar que resposta do cartao seja diferente
; de #00
; Destroi A, BC
; ------------------------------------------------
WAIT_RESP_NO_00:
	ld	bc, 128		; 32768 tentativas
.loop:
	ld	a, (PORTSPI)
	or	a
	ret	nz			; se resposta for <> #00, sai
	djnz	.loop
	dec	c
	jr	nz,.loop
	scf			; erro
	ret

; ------------------------------------------------
; Ativa (seleciona) cartao atual baixando seu /CS
; Modify: AF
; ------------------------------------------------
setaSDAtual:
	ld	a, (PORTSPI)	; dummy read
	ld	a, (iy+NUMSD)
	cpl			; inverte bits
	ld	(PORTCFG), a
	ret

; ------------------------------------------------
; Grava um bloco de 512 bytes no cartao
; HL aponta para o inicio dos dados
; BC e DE contem o numero do bloco (BCDE = 32 bits)
; Modifies:  AF, BC, DE, HL, IXL
; ------------------------------------------------
GravarBloco:
	dec	ixl		; SDcard V1?
	call	m,blocoParaByte	; Yes, convert blocks to bytes
	call	setaSDAtual	; selecionar cartao atual
	ld	a,ixh		; get Number of blocks to write
	dec	a
	jp	z,.umBloco	; somente um bloco, gravar usando CMD24

; multiplos blocos
	exx
	ld	a, CMD55	; Multiplos blocos, mandar ACMD23 com total de blocos
	call	SD_SEND_CMD_NO_ARGS
	ld	a, ACMD23
	ld	bc, 0
	ld	d, c
	ld	e,ixh		; e=Number of blocks to write
	call	SD_SEND_CMD
	or	a
	jr	nz,.erroEscritaBlocoR	; erro no ACMD23
	exx
	ld	a, CMD25	; CMD25 = write multiple blocks
	call	SD_SEND_CMD
	or	a
	jr	nz,.erroEscritaBlocoR	; erro

	; Check for a Z80 or R800
	ex	de,hl
;	ld	a,20
	dec	a		; a=#FF
	db	#ED,#F9		; mulub a,a
	ex	de,hl
	jr	nc,.loopZ80	; Use the Z80 optimized loop

	exx
	call	GTR800LDIR
	exx			; hl'=Pointer to R800LDIR helper routine
.loopR800:	; R800 optimized loop
	ld	de,PORTSPI
	ld	a, #FC		; mandar #FC para indicar que os proximos dados
	ld	(de),a		; sao para gravacao
	call	R800LDIR
	ld	(PORTSPI),a	; Send a dummy 16bit CRC
	ld	(PORTSPI),a
; Wait for !FFh
	ld	b,5		; 5*5ms = 250ms
.rwaitnoFFext:
	in	a,(#E7)
	ld	c,a
.rwaitnoFF:
	ld	a,(PORTSPI)
	cp	#FF		; a=FF?
	jr	nz,.rnoFFok
	in	a,(#E7)
	sub	c
	cp	50		; 50mS?
	jr	c,.rwaitnoFF	; wait 50ms
	djnz	.rwaitnoFF	; x50ms wait
	jr	.erroEscritaBlocoR	; error: timeout
.rnoFFok:
	and	#1F		; testa bits erro
	cp	5
	jr	nz,.erroEscritaBlocoR	; resposta errada, informar erro
; Wait for !00h
	ld	b,5		; 5*50ms = 250ms
.rwaitno00ext:
	in	a,(#E7)
	ld	c,a
.rwaitno00:
	ld	a,(PORTSPI)
	or	a
	jr	nz,.rno00ok
	in	a,(#E7)
	sub	c
	cp	50		; 50ms
	jr	c,.rwaitno00	; wait 50ms
	djnz	.rwaitno00ext	; x50 ms wait
.erroEscritaBlocoR: ; Trick to save some cycles inside the block transfer loop
	scf
	jp	terminaLeituraEscritaBloco
.rno00ok:
	dec	ixh		; nblocks=nblocks-1
	jr	nz,.loopR800
	jp	.loopend


.loopZ80:	; Z80 optimized loop
	ld	de,PORTSPI
	ld	a, #FC		; mandar #FC para indicar que os proximos dados
	ld	(de),a		; sao para gravacao
 rept 512
	ldi		; 512 vezes o opcode LDI
 endm
	ld	(PORTSPI),a	; Send a dummy 16bit CRC
	ld	(PORTSPI),a
	call	WAIT_RESP_NO_FF	; esperar cartao
	and	#1F		; testa bits erro
	cp	5
	jr	nz,.erroEscritaBlocoZ	; resposta errada, informar erro
	call	WAIT_RESP_NO_00	; esperar cartao
	jr	c,.erroEscritaBlocoZ
	dec	ixh		; nblocks=nblocks-1
	jp	nz,.loopZ80
.loopend:
	ld	a, (PORTSPI)	; acabou os blocos, fazer 2 dummy reads
	ld	a, (PORTSPI)
	ld	a, #FD		; enviar #FD para informar ao cartao que acabou os dados
	ld	(PORTSPI),a
	ld	a, (PORTSPI)	; dummy reads
	ld	a, (PORTSPI)
	call	WAIT_RESP_NO_00	; esperar cartao
	jp	.fim		; CMD25 concluido, sair informando nenhum erro



.r800s:
	exx
	call	GTR800LDIR
	exx			; hl'=Pointer to R800LDIR helper routine
	call	R800LDIR
	jp	.part2s

.erroEscritaBlocoZ: ; Trick to save some cycles inside the block transfer loop
	scf
	jp	terminaLeituraEscritaBloco

.umBloco:
	ld	a, CMD24	; CMD24 = Write Single Block
	call	SD_SEND_CMD_GET_ERROR
	jp	c,terminaLeituraEscritaBloco	; erro


	ld	a, #FE		; mandar #FE para indicar que vamos mandar dados para gravacao
	ld	(PORTSPI),a

	; Check for a Z80 or R800
	ex	de, hl
;	ld	a,20
	db	#ED,#F9		; mulub a,a
	ex	de, hl
	ld	de,PORTSPI
	jr	c,.r800s	; Use LDIR for R800
 rept 512
	ldi
 endm
.part2s:
	ld	a, #FF		; envia dummy CRC
	ld	(PORTSPI),a
	ld	(PORTSPI),a
	call	WAIT_RESP_NO_FF	; esperar cartao
	and	#1F		; testa bits erro
	cp	5
	scf
	jp	nz,terminaLeituraEscritaBloco	; resposta errada, informar erro
.esp:
	call	WAIT_RESP_NO_FF	; esperar cartao
	or	a
	jr	z,.esp
.fim:
	xor	a		; zera carry e informa nenhum erro
terminaLeituraEscritaBloco:
	push	af
	call	desabilitaSDs	; desabilitar todos os cartoes
	pop	af
	ret


; ------------------------------------------------
; Ler um bloco de 512 bytes do cartao
; HL aponta para o inicio dos dados
; BC e DE contem o numero do bloco (BCDE = 32 bits)
; Destroi AF, BC, DE, HL, IXL
; ------------------------------------------------
LerBloco:
	dec	ixl		; SDcard V1?
	call	m,blocoParaByte	; Yes, convert blocks to bytes
	call	setaSDAtual
	ld	a,ixh		; get Number of blocks to write
	dec	a
	jp	z,.umBloco	; somente um bloco, pular

; multiplos blocos
	ld	a, CMD18	; CMD18 = Read Multiple Blocks
	call	SD_SEND_CMD_GET_ERROR
	jr	c,terminaLeituraEscritaBloco

	ex	de,hl		; de=destination address
	; Check for a Z80 or R800
;	ld	a,20
	dec	a		; a=FFh
	db	#ED,#F9		; mulub a,a
	jr	nc,.loopZ80	; Use the Z80 optimized loop

	exx
	call	GTR800LDIR
	exx			; hl'=Pointer to R800LDIR helper routine
.loopR800:
	ld	b,0
	ld	a,(#E7)
	ld	c,a
.rwaitFE:
	ld	a,(PORTSPI)
	cp	#FE
	jr	z,.rFEok
	djnz	.rwaitFE	; fast card wait
	ld	a,(#E7)
	sub	c
	cp	100		; 100mS?
	jr	c,.rwaitFE	; slow card wait
	scf
	jr	terminaLeituraEscritaBloco
.rFEok:
	ld	hl,PORTSPI
	call	R800LDIR
	ld	a, (PORTSPI)	; discard 16bit CRC
	ld	a, (PORTSPI)
	dec	ixh		; nblocks=nblocks-1
	jr	nz,.loopR800
	jp	.loopend

.loopZ80:
	ld	bc,0
.zwaitFE:
	ld	a,(PORTSPI)
	cp	#FE
	jr	z,.zFEok
	djnz	.zwaitFE	; fast card wait
	ex	(sp),hl
	ex	(sp),hl
	dec	c
	jr	nz,.zwaitFE	; slow card wait
	scf
	jr	terminaLeituraEscritaBloco
.zFEok:
	ld	hl,PORTSPI
 rept 512
	ldi
 endm
	ld	a, (PORTSPI)	; discard 16bit CRC
	ld	a, (PORTSPI)
	dec	ixh		; nblocks=nblocks-1
	jp	nz,.loopZ80
.loopend:
	ld	a, CMD12	; acabou os blocos, mandar CMD12 para cancelar leitura
	call	SD_SEND_CMD_NO_ARGS
	jp	.fim

.r800s:
	exx
	call	GTR800LDIR
	exx			; hl'=Pointer to R800LDIR helper routine
	call	R800LDIR
	jp	.part2s

.umBloco:
	ld	a, CMD17	; CMD17 = Read Single Block
	call	SD_SEND_CMD_GET_ERROR
	jp	c,terminaLeituraEscritaBloco

	call	WAIT_RESP_FE
	jp	c,terminaLeituraEscritaBloco
	ex	de,hl

	; Check for a Z80 or R800
;	ld	a,20
	db	#ED,#F9		; mulub a,a
	ld	hl,PORTSPI
	jr	c,.r800s	; Use LDIR for R800
 rept 512
	ldi
 endm
.part2s:
	ld	a, (PORTSPI)	; discard 16bit CRC
	ld	a, (PORTSPI)
.fim:
	xor	a		; zera carry para informar leitura sem erros
	jp	terminaLeituraEscritaBloco


; ------------------------------------------------
; Converte blocos para bytes. Na pratica faz
; BC DE = (BC DE) * 512
; ------------------------------------------------
blocoParaByte:
	ld	b, c
	ld	c, d
	ld	d, e
	ld	e, 0
	sla	d
	rl	c
	rl	b
	ret

; ------------------------------------------------
R800LDIR:
	exx
	jp	(hl)

; ------------------------------------------------


; ==========================================================================
; Funcoes utilitarias
; ==========================================================================

; ------------------------------------------------
; Chaveia para modo SPI
; ------------------------------------------------
modoSPI:
	push	af
	ld	a, 1
	ld	(CHAVEIASPI), a
	pop	af
	ret

; ------------------------------------------------
; Chaveia para modo ROM
; ------------------------------------------------
modoROM:
	push	af
	ld	a, 0
	ld	(CHAVEIASPI), a
	pop	af
	ret

; ------------------------------------------------
; Imprime string na tela apontada por DE
; Destroi todos os registradores
; ------------------------------------------------
printString:
	ld	a, (de)
	or	a
	ret	z
	call	CHPUT
	inc	de
	jr	printString


; ------------------------------------------------
; Converte o byte em A para string em decimal no
; buffer apontado por DE
; Destroi AF, BC, HL, DE, IXL
; ------------------------------------------------
DecToAscii:
	ld	h, 0
	ld	l, a		; copiar A para HL
	ld	(iy+TEMP),1	; flag para indicar que devemos cortar os zeros a esquerda
	ld	bc, -100	; centenas
	call	.num1
	ld	c, -10		; dezenas
	call	.num1
	ld	(iy+TEMP),2	; unidade deve exibir 0 se for zero e nao corta-lo
	ld	c, -1		; unidades
.num1:
	ld	a, '0'-1
.num2:
	inc	a		; contar o valor em ascii de '0' a '9'
	add	hl, bc		; somar com negativo
	jr	c,.num2		; ainda nao zeramos
	sbc	hl, bc		; retoma valor original
	dec	(iy+TEMP)	; se flag do corte do zero indicar para nao cortar, pula
	jr	nz,.naozero
	cp	'0'		; devemos cortar os zeros a esquerda. Eh zero?
	jr	nz,.naozero
	inc	(iy+TEMP)	; se for zero, nao salvamos e voltamos a flag
	ret
.naozero:
	ld	(de), a		; eh zero ou eh outro numero, salvar
	inc	de		; incrementa ponteiro de destino
	ret

; ------------------------------------------------
; Converte o byte em A para string em hexa no
; buffer apontado por DE
; Destroi AF, C, DE
; ------------------------------------------------
HexToAscii:
	ld	c, a
	rra
	rra
	rra
	rra
	call	.conv
	ld  	a, c
.conv:
	and	#0F
	add	a, #90
	daa
	adc	a, #40
	daa
	ld	(de), a
	inc	de
	ret

; ------------------------------------------------
; Converte o byte em A para string em decimal e
; imprime na tela
; Destroi AF, BC, HL, DE
; ------------------------------------------------
printDecToAscii:
	ld	h, 0
	ld	l, a		; copiar A para HL
	ld	b, 1		; flag para indicar que devemos cortar os zeros a esquerda
	ld	de, -100	; centenas
	call	.num1
	ld	e, -10		; dezenas
	call	.num1
	ld	b, 2		; unidade deve exibir 0 se for zero e nao corta-lo
	ld	e, -1		; unidades
.num1:
	ld	a, '0'-1
.num2:
	inc	a		; contar o valor em ascii de '0' a '9'
	add	hl, de		; somar com negativo
	jr	c,.num2		; ainda nao zeramos
	sbc	hl, de		; retoma valor original
	djnz	.naozero	; se flag do corte do zero indicar para nao cortar, pula
	cp	'0'		; devemos cortar os zeros a esquerda. Eh zero?
	jr	nz,.naozero
	inc	b		; se for zero, nao imprimimos e voltamos a flag
	ret
.naozero:
	push	hl		; nao eh zero ou eh outro numero, imprimir
	push	bc
	call	CHPUT
	pop	bc
	pop	hl
	ret

; ------------------------------------------------
; Procura pelo nome do fabricante em uma tabela.
; A contem o byte do fabricante
; Devolve HL apontando para o buffer do fabricante
; e BC com o comprimento do texto
; Destroi AF, BC, HL
; ------------------------------------------------
pegaFabricante:
	ld	c, a
	ld	hl, tblFabricantes

.loop:
	ld	a, (hl)
	inc	hl
	cp	c
	jr	z,.achado
	or	a
	jr	z,.achado
	push	bc
	call	.achado
	add	hl, bc
	inc	hl
	pop	bc
	jr	.loop

.achado:
	ld	c, 0
	push	hl
	xor	a
.loop2:
	inc	c
	inc	hl
	cp	(hl)
	jr	nz,.loop2
	pop	hl
	ld	b, 0
	ret

tblFabricantes:
	db	1
	db	"Panasonic",0
	db	2
	db	"Toshiba",0
	db	3
	db	"SanDisk",0
	db	4
	db	"SMI-S",0
	db	6
	db	"Renesas",0
	db	17
	db	"Dane-Elec",0
	db	19
	db	"KingMax",0
	db	21
	db	"Samsung",0
	db	24
	db	"Infineon",0
	db	26
	db	"PQI",0
	db	27
	db	"Sony",0
	db	28
	db	"Transcend",0
	db	29
	db	"A-DATA",0
	db	31
	db	"SiliconPower",0
	db	39
	db	"Verbatim",0
	db	65
	db	"OKI",0
	db	115
	db	"SilverHT",0
	db	137
	db	"L.Data",0
	db	0
	db	"Generico",0


; ------------------------------------------------
; Restore screen parameters on MSX>=2 if they're
; not set yet
; ------------------------------------------------
MYSETSCR:
	ld	a,(MSXVER)
	or	a			; MSX1?
	jp	z,INITXT		; Yes, change do screen-0

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
; Install the R800 data transfer helper routine on WorkArea
; ------------------------------------------------
INSTR800HLP:
	ld	a,(MSXVER)
	cp	3		; MSX Turbo-R?
	ret	c		; No, return
	call	GTR800LDIR
	ex	de,hl
	ld	hl,R800DATHLP
	ld	bc,R800DATHLP.end-R800DATHLP
	ldir
	ret

; ------------------------------------------------
; Obtain the pointer to the R800 data transfer helper routine
; Input   : IY=Pointer to the WorkArea
; Output  : HL=pointer to R800 data transfer helper routine
; Modifies: DE
; ------------------------------------------------
GTR800LDIR:
	push	iy
	pop	hl			; hl=WorkArea
	ld	de,TRLDIR
	add	hl,de
	ret

; ------------------------------------------------
; R800 data transfer routine, copied to the WorkArea
; ------------------------------------------------
R800DATHLP:
	exx
	ld	bc,512
	ldir
	ret
.end:


; ==========================================================================
strTitle:
	db	"FBLabs SDHC driver "
	db	"v",VER_MAIN+#30,'.',VER_SEC+#30,'.',VER_REV+#30
	db	13,10,0

strBootpaused:
	db	"Paused. Press <i> to show the copyright info.",13,10,0

strCopyright:
	db	"(c) 2014 Fabio Belavenuto",13,10
	db	"(c) 2016 FRS",13,10
	db	"Licensed under CERN OHL v1.1",13,10
	db	"http://ohwr.org/cernohl",13,10
	db	"PCB designed by Luciano Sturaro",13,10
	; fall throw
strCrLf:
	db	13,10,0
strCartao:
	db	"Slot ",0
strVazio:
	db	"Empty",13,10,0
strNaoIdentificado:
	db	"Unknown!",13,10,0
			;----------------------------------------
strMr_mp_desativada:
	db	"Slot expander/Mapper/MegaRAM disabled",13,10,0
strMapper:
	db	"Slot expanded and Memory Mapper enabled",13,10,0
strMegaram:
	db	"Slot expander and MegaRAM enabled",13,10,0
strSDV1:
	db	"SDV1 - ",0
strSDV2:
	db	"SDV2 - ",0

;-----------------------------------------------------------------------------
;
; End of the driver code

DRV_END:

	ds	3ED0h-(DRV_END-DRV_START), #FF

