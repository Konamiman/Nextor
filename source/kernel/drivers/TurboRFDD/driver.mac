	.z80
	title	Nextor v3 Driver for MSX Turbo-R FDD (TC8566AF)
	subttl	Floppy disk driver for FS-A1GT/FS-A1ST

;=============================================================================
;
; Nextor v3 driver for the floppy disk drive built into MSX Turbo-R computers.
; Drives the TC8566AF FDC and uses S1990 hardware for disk change detection.
; Assumes ASCII16 mapper for the Nextor ROM. FDC hardware is in slot 3-2.
;
; The Nextor ROM is in a different slot than the FDC hardware.
; All FDC register accesses go through RDSLT/WRSLT (BIOS inter-slot I/O).
; The time-critical 512-byte data transfer runs from a RAM routine in page 3
; that temporarily switches page 1 to slot 3-2 for direct FDC access.
;
;=============================================================================
;
; HARDWARE SPECIFICATION
; ======================
;
; All registers are memory-mapped in slot 3-2. The FDC is a TC8566AF,
; a µPD765-compatible controller with integrated DOR/TDR registers.
; The S1990 is the Turbo-R system controller.
;
;
; MEMORY-MAPPED REGISTERS
; -----------------------
;
; 7FF1h - S1990 Register (read only, slot 3-2)
;
;   The S1990 system controller provides disk change detection signals.
;   Only bits 4-5 are relevant to the FDD:
;
;   Bit 5: ~DC1 - Disk change drive 1 (active low: 0=changed, 1=not changed)
;   Bit 4: ~DC0 - Disk change drive 0 (active low: 0=changed, 1=not changed)
;
;   Note: The disk change signal is latched by the FDC and only updates
;   after a SENSE DRIVE STATUS command is issued.
;
;
; 7FF2h - DOR: Digital Output Register (write only)
;
;   Controls motor, drive select, and FDC reset.
;
;   Bit 5: MTR1 - Motor on for drive 1 (1=on, 0=off)
;   Bit 4: MTR0 - Motor on for drive 0 (1=on, 0=off)
;   Bit 3: (unused)
;   Bit 2: FRST - FDC enable (1=normal operation, 0=reset)
;   Bit 1: DS1  - Drive select bit 1
;   Bit 0: DS0  - Drive select bit 0
;
;   Typical values:
;     00h - Full reset (FDC disabled, motors off)
;     04h - FDC enabled, all motors off
;     14h - FDC enabled, motor on, drive 0 selected
;     25h - FDC enabled, motor on, drive 1 selected
;
;
; 7FF3h - TDR: TC/READY Control Register (write only)
;
;   Multiplexes the Terminal Count and READY signals, which on standard
;   FDC implementations are dedicated hardware lines from the DMA controller
;   and drive, respectively. On the Turbo-R these are software-controlled.
;
;   Bit 5: READY enable (allows READY signal input)
;   Bit 4: Force READY high (when bit 5 is also set)
;   Bit 1: TC enable (must be set for TC pulse to take effect)
;   Bit 0: TC signal level (pulse 0→1→0 to terminate data transfer)
;
;   Typical values:
;     FAh - Initialization: force READY high, TC enabled
;     20h - Let READY come from actual drive signal (for polling drive status)
;     30h - Force READY high (after status check, for normal operation)
;     02h - TC idle (TC enabled, TC=0)
;     03h - TC active (TC enabled, TC=1; used in 02h→03h→02h pulse sequence)
;
;
; 7FF4h - MSR: Main Status Register (read only)
;
;   Standard µPD765-compatible status register.
;
;   Bit 7: RQM  - Request for Master (1=FDC ready for CPU data transfer)
;   Bit 6: DIO  - Data direction (1=FDC→CPU read, 0=CPU→FDC write)
;   Bit 5: EXM  - Execution mode (1=in execution phase, data transfer active)
;   Bit 4: CB   - Command busy (1=command in progress)
;   Bit 3: DB3  - Drive 3 seeking
;   Bit 2: DB2  - Drive 2 seeking
;   Bit 1: DB1  - Drive 1 seeking
;   Bit 0: DB0  - Drive 0 seeking
;
;
; 7FF5h - DATA: Data Register (read/write)
;
;   Standard µPD765-compatible data register. Used for:
;   - Writing command bytes (CPU→FDC when MSR.DIO=0 and MSR.RQM=1)
;   - Reading result bytes (FDC→CPU when MSR.DIO=1 and MSR.RQM=1)
;   - Reading/writing sector data during execution phase (MSR.EXM=1)
;
;
; FDC COMMANDS USED
; -----------------
;
; All commands are µPD765-compatible.
; Commands marked (MFM) have bit 6 set in the command byte for MFM encoding.
;
; SPECIFY (03h) - 3 command bytes, no result bytes
;   Bytes: 03h, [SRT:4|HUT:4], [HLT:7|ND:1]
;   Sets step rate time, head unload time, head load time, and DMA mode.
;   This driver uses: SRT=3ms (0Dh), HUT=240ms (0Fh), HLT=2ms (01h), ND=1 (non-DMA).
;   Combined as: 03h, DFh, 03h.
;
; SENSE DRIVE STATUS (04h) - 2 command bytes, 1 result byte
;   Bytes: 04h, [xxxHDxUS1US0]
;   Returns: ST3
;   Used to poll drive READY status and check write protect.
;   ST3 bit 5 = READY, bit 6 = write protected.
;   Also latches the disk change signal in the S1990 register.
;
; RECALIBRATE (07h) - 2 command bytes, no result bytes (interrupt-driven)
;   Bytes: 07h, [xxxxxUS1US0]
;   Seeks to track 0. Completion signaled via SENSE INTERRUPT STATUS.
;
; SENSE INTERRUPT STATUS (08h) - 1 command byte, 2 result bytes
;   Bytes: 08h
;   Returns: ST0, current cylinder number
;   Must be issued after SEEK/RECALIBRATE to acknowledge completion.
;   ST0 bit 5 = seek end, bits 7-6 = 00 normal termination.
;
; SEEK (0Fh) - 3 command bytes, no result bytes (interrupt-driven)
;   Bytes: 0Fh, [xxxxxUS1US0], cylinder
;   Seeks to specified cylinder. Completion signaled via SENSE INTERRUPT STATUS.
;
; READ DATA (46h, MFM) - 9 command bytes, 7 result bytes
;   Bytes: 46h, [xxxHDxUS1US0], C, H, R, N, EOT, GPL, DTL
;   C=cylinder, H=head, R=sector (1-based), N=sector size code (2=512),
;   EOT=last sector number on track, GPL=gap length (80), DTL=FFh.
;   Returns: ST0, ST1, ST2, C, H, R, N
;   Data phase: 512 bytes FDC→CPU, terminated by TC pulse.
;
; WRITE DATA (45h, MFM) - 9 command bytes, 7 result bytes
;   Same parameter format as READ DATA.
;   Data phase: 512 bytes CPU→FDC, terminated by TC pulse.
;
;
; PROCEDURES
; ----------
;
; FDC Initialization:
;   1. Reset FDC: write 00h to DOR (FRST=0)
;   2. Initialize TC/READY: write FAh to TDR (force READY high, TC enabled)
;   3. Enable FDC: write 04h to DOR (FRST=1, motors off)
;   4. Send SPECIFY command: 03h, DFh, 03h (SRT=3ms, HUT=240ms, HLT=2ms, non-DMA)
;
; Drive Detection (for each drive):
;   1. Turn on motor: write DOR (14h for drive 0, 25h for drive 1)
;   2. Wait for motor spinup (~0.5s busy-wait loop)
;   3. Send RECALIBRATE command (07h, unit)
;   4. Wait for seek complete:
;      a. Poll MSR bit 4 until not busy
;      b. Issue SENSE INTERRUPT STATUS until ST0 bit 5 (seek end) is set
;      c. Check ST0 bits 7-6 for normal termination (00)
;   5. If seek completes without error, drive is present
;
; Reading/Writing a Sector:
;   1. Turn on motor via DOR
;   2. Check drive ready:
;      a. Write 20h to TDR (let READY come from drive)
;      b. Poll with SENSE DRIVE STATUS until ST3 bit 5 (READY) is set
;      c. Write 30h to TDR (force READY high for normal operation)
;   3. For writes: check ST3 bit 6 (write protect)
;   4. Seek to target cylinder (with recalibrate if track 0)
;   5. Send READ DATA (46h) or WRITE DATA (45h) command (9 bytes)
;   6. Execute data transfer via page 3 RAM routine:
;      a. Disable interrupts (di)
;      b. Switch page 1 (4000h-7FFFh) to slot 3-2 for direct FDC register access
;      c. Poll MSR bit 7 (RQM) for each byte; check bit 5 (EXM) for end of phase
;      d. Read from / write to FDC_DAT (7FF5h) directly (no inter-slot overhead)
;      e. After all data transferred, send TC pulse: write 02h, 03h, 02h to TDR
;      f. Restore page 1 to original slot, enable interrupts (ei)
;   7. Read 7 result bytes (ST0-ST2, C, H, R, N) via inter-slot access
;   8. On error: reposition (seek track 6, recalibrate, seek back) and retry
;      up to 11 times (reposition only on even-numbered retries)
;   9. Set motor timeout counter (~1 second); motor turned off by timer interrupt
;
; CHS Calculation (from logical sector number):
;   - sector_in_track = logical_sector MOD sectors_per_track (1-based for FDC)
;   - track = logical_sector / sectors_per_track
;   - For double-sided: cylinder = track / 2, head = track MOD 2
;   - For single-sided: cylinder = track, head = 0
;   - Sectors per track determined by media descriptor bit 1 (0=9spt, 1=8spt)
;   - Double/single sided determined by media descriptor bit 0 (1=DS, 0=SS)
;
; Disk Change Detection:
;   1. Send SENSE DRIVE STATUS command (04h, unit) to latch change signal
;   2. Wait for FDC result phase (poll MSR bit 7)
;   3. Read S1990 register at 7FF1h in slot 3-2 via RDSLT
;   4. Check bit 4 (drive 0) or bit 5 (drive 1)
;   5. Signal is active low: 0 = disk was changed, non-zero = not changed
;   6. Read FDC result bytes to complete the command
;
;=============================================================================

;--- Query result codes

QUERY_OK: equ 0
QUERY_TRUNCATED_STRING: equ 1
QUERY_INVALID_DEVICE: equ 2
QUERY_INIT_ERROR: equ 3
QUERY_NOT_IMPLEMENTED: equ 0FFh

;--- MSX-DOS error codes for read/write operations

_NCOMP:	equ	0FFh
_WRERR:	equ	0FEh
_DISK:	equ	0FDh
_NRDY:	equ	0FCh
_DATA:	equ	0FAh
_RNF:	equ	0F9h
_WPROT:	equ	0F8h
_UFORM:	equ	0F7h
_SEEK:	equ	0F3h
_IFORM:	equ	0F0h
_IDEVN:	equ	0B5h
_IPARM:	equ	08Bh

;--- TC8566AF FDC registers (memory-mapped in slot 3-2)

FDC_DOR:	equ	7FF2h		;Digital Output Register (write)
FDC_TDR:	equ	7FF3h		;TC/READY control (write)
FDC_MSR:	equ	7FF4h		;Main Status Register (read)
FDC_DAT:	equ	7FF5h		;Data Register (read/write)

;--- S1990 register

S1990:	equ	7FF1h		;bit 4 = ~DC0, bit 5 = ~DC1

;--- BIOS entry points

RDSLT:	equ	000Ch		;Read byte from slot: A=slot,HL=addr -> A=byte
WRSLT:	equ	0014h		;Write byte to slot: A=slot,HL=addr,E=byte

;--- Hardware constants

FDC_SLOT:	equ	8Bh		;Slot 3-2 (primary 3, secondary 2, expanded)

;--- Work area offsets (relative to IX)

WK_NDRV:	equ	0		;Number of physical drives detected
WK_MT0:	equ	1		;Motor timeout counter drive 0
WK_MT1:	equ	2		;Motor timeout counter drive 1
FMT_SECNUM:	equ	3		;2 bytes: sector number for format init
WK_FLAGS:	equ	5		;bit 0 = write operation
WK_CMD:	equ	10		;FDC command buffer (9 bytes, +10..+18)
WK_RES:	equ	19		;FDC result buffer (7 bytes, +19..+25)
WK_XFER:	equ	26		;Start of RAM transfer routine code

WK_XFER_SIZE:	equ 52		;Space reserved for RAM transfer routine

;--- Format state (after XFER code area)

FMT_SIDES:	equ	WK_XFER+WK_XFER_SIZE	;Number of sides (1 or 2)
FMT_DRIVE:	equ	FMT_SIDES+1		;0-based drive number
FMT_TRACK:	equ	FMT_DRIVE+1		;Current track number
FMT_SIDE:	equ	FMT_TRACK+1		;Current side (0 or 1)

WK_SIZE:	equ	FMT_SIDE+1

MOTOR_TIMEOUT:	equ 60		;~1 second at 60Hz VDP interrupt

;--- Kernel page 0 routines

GWORK:	equ	4045h
CALBNK:	equ	4042h


;*********************
;***  DRIVER CODE  ***
;*********************

	org	4100h

DRIVER_START:

	;--- Driver signature

	db	"NEXTORv3_DRIVER",0

	;--- Jump table

	jp	TIMER_INT
	jp	OEMSTAT
	jp	BASDEV
	jp	EXTBIO
	jp	DIRECT_0
	jp	DIRECT_1
	jp	DIRECT_2
	jp	DIRECT_3
	jp	DIRECT_4
	jp	DRIVER_QUERY
	jp	DEVICE_QUERY
	jp	CUSTOM_DRIVER_QUERY
	jp	CUSTOM_DEVICE_QUERY
	jp	READ_WRITE


;=============================================================================
;  TIMER INTERRUPT
;=============================================================================

TIMER_INT:
	call	MY_GWORK
	ld	a,(ix+WK_MT0)
	or	a
	jr	z,TI_CHK1
	dec	(ix+WK_MT0)
	jr	nz,TI_CHK1
	call	MOTOR_OFF
TI_CHK1:
	ld	a,(ix+WK_MT1)
	or	a
	ret	z
	dec	(ix+WK_MT1)
	ret	nz
	jp	MOTOR_OFF


;=============================================================================
;  STUBS
;=============================================================================

OEMSTAT:
	scf
	ret
	ret

BASDEV:
	scf
	ret
	ret

EXTBIO:
	ret
	ret
	ret

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

CUSTOM_DRIVER_QUERY:
	ld	a,QUERY_NOT_IMPLEMENTED
	ret

CUSTOM_DEVICE_QUERY:
	ld	a,QUERY_NOT_IMPLEMENTED
	ret


;=============================================================================
;  DRIVER QUERY
;=============================================================================

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
	ld	a,QUERY_NOT_IMPLEMENTED
	ret


;--- Get driver version: 1.0.0

DO_DRVQ_GET_VERSION:
	ld	bc,0100h
	ld	d,0
	xor	a
	ret


;--- Get driver string

DO_DRVQ_GET_STRING:
	ld	a,b
	ld	b,d
	ex	de,hl
	dec	a
	ld	hl,STR_DRV_NAME
	jp	z,OUTPUT_STRING
	dec	a
	ld	hl,STR_DRV_AUTHOR
	jp	z,OUTPUT_STRING
	dec	a
	ld	hl,STR_HW_NAME
	jp	z,OUTPUT_STRING
	dec	a
	ld	hl,STR_HW_AUTHOR
	jp	z,OUTPUT_STRING
	ld	a,QUERY_NOT_IMPLEMENTED
	ret


;--- Get init params: request work area and timer hook

DO_DRVQ_GET_INIT_PARAMS:
	xor	a
	ld	b,1		;Request timer interrupt hook
	ld	hl,WK_SIZE
	ret


;--- Initialize driver: detect drives and init FDC

DO_DRVQ_INIT:
	;Print init message (DE = CHPUT from kernel)
	push	de		;Save CHPUT for error printing
	ld	hl,INIT_MSG
	call	PRINT_WITH_DE

	;Get work area and clear it
	call	MY_GWORK

	push	ix
	pop	hl
	ld	b,WK_XFER	;Only clear data portion, not code area
	xor	a
INIT_CLR:
	ld	(hl),a
	inc	hl
	djnz	INIT_CLR

	;Copy RAM transfer routine to work area
	push	ix
	pop	hl
	ld	de,WK_XFER
	add	hl,de
	ex	de,hl		;DE = destination (work area + WK_XFER)
	ld	hl,XFER_CODE_START
	ld	bc,XFER_CODE_SIZE
	ldir

	;Initialize FDC
	call	FDC_INIT
	jr	c,INIT_FDC_ERR

	;Detect drive 0 (always present on Turbo-R)
	ld	a,14h		;Motor on drive 0, FDC enabled
	call	DETECT_DRIVE
	jr	c,INIT_NO_DRV

	ld	(ix+WK_NDRV),1

	;Try drive 1 (external)
	ld	a,25h		;Motor on drive 1, FDC enabled
	call	DETECT_DRIVE
	jr	c,INIT_DONE

	ld	(ix+WK_NDRV),2

INIT_DONE:
	call	MOTOR_OFF
	ld	a,(ix+WK_NDRV)
	add	a,'0'
	ld	b,a		;B = ASCII digit
	pop	de		;DE = CHPUT

	push	bc		;Save digit
	ld	hl,STR_NDRV_1
	call	PRINT_WITH_DE	;Print "Found "
	pop	bc

	;Output digit character via CHPUT trampoline
	push	de
	ld	de,0C300h	;JP opcode + 00
	push	de
	ld	ix,1
	add	ix,sp		;IX -> JP <CHPUT> on stack
	ld	a,b
	call	JP_IX		;Print digit
	pop	de
	pop	de

	ld	hl,STR_NDRV_2
	call	PRINT_WITH_DE	;Print " drive(s)"
	xor	a		;QUERY_OK
	ret

INIT_FDC_ERR:
	call	MOTOR_OFF
	pop	de		;DE = CHPUT
	ld	hl,STR_ERR_FDC
	call	PRINT_WITH_DE
	ld	a,QUERY_INIT_ERROR
	ret

INIT_NO_DRV:
	call	MOTOR_OFF
	pop	de		;DE = CHPUT
	ld	hl,STR_ERR_NODRIVE
	call	PRINT_WITH_DE
	ld	a,QUERY_INIT_ERROR
	ret


;--- Detect a drive: turn on motor, wait, recalibrate
;    Input: A = DOR value, IX = work area
;    Output: Cy=0 found, Cy=1 not found

DETECT_DRIVE:
	push	af		;Save DOR value
	and	1		;Extract drive select bit
	ld	(ix+WK_CMD+1),a
	pop	af

	call	WRITE_DOR	;Turn on motor via inter-slot write

	;Wait for motor spinup
	ld	bc,0CDE5h
DD_WAIT:
	ex	(sp),hl
	ex	(sp),hl
	dec	bc
	ld	a,b
	or	c
	jr	nz,DD_WAIT

	;Recalibrate
	ld	(ix+WK_CMD),07h
	ld	b,2
	call	FDC_WRITE_CMD
	ret	c
	jp	FDC_WAIT_SEEK


;--- Get max device number

DO_DRVQ_GET_MAX_DEVICE:
	call	MY_GWORK
	ld	b,(ix+WK_NDRV)
	xor	a
	ret


;=============================================================================
;  DEVICE QUERY
;=============================================================================

DEVICE_QUERY:
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
	ld	a,QUERY_NOT_IMPLEMENTED
	ret


;--- Stop motor

DO_DEVQ_STOP_MOTOR:
	call	CHECK_DEVICE
	ret	nz
	call	MOTOR_OFF
	xor	a
	ret


;--- Get device string

DO_DEVQ_GET_STRING:
	call	CHECK_DEVICE
	ret	nz
	ld	a,b
	ld	b,d
	ex	de,hl
	ld	hl,STR_DEV_NAME
	cp 2 ;Medium name
	jp	z,OUTPUT_STRING
	cp 4 ;Device name
	jp	z,OUTPUT_STRING
DEVS_NOTIMP:
	ld	a,QUERY_NOT_IMPLEMENTED
	ret


;--- Get device parameters

DO_DEVQ_GET_PARAMS:
	call	CHECK_DEVICE
	ret	nz
	ld	a,h
	or	l
	jr	z,DEVP_NOINFO
	ld	(hl),0		;+0: block device
	inc	hl
	ld	(hl),0		;+1: sector size low = 0
	inc	hl
	ld	(hl),2		;+2: sector size high = 2 (512)
	inc	hl
	xor	a
	ld	(hl),a		;+3..+6: total sectors = 0 (unknown)
	inc	hl
	ld	(hl),a
	inc	hl
	ld	(hl),a
	inc	hl
	ld	(hl),a
	inc	hl
	ld	(hl),05h	;+7: flags = removable + floppy
	inc	hl
	ld	(hl),80		;+8: cylinders low = 80
	inc	hl
	ld	(hl),0		;+9: cylinders high = 0
	inc	hl
	ld	(hl),2		;+10: heads = 2
	inc	hl
	ld	(hl),9		;+11: sectors per track = 9
DEVP_NOINFO:
	xor	a
	ret


;--- Get device status (disk change via S1990)

DO_DEVQ_GET_STATUS:
	call	CHECK_DEVICE
	ret	nz

	push	ix
	call	MY_GWORK

	;Determine S1990 bit: drive 0 = bit 4, drive 1 = bit 5
	ld	a,c
	dec	a		;0-based unit
	ld	b,10h
	jr	z,DSTAT_BIT
	ld	b,20h
DSTAT_BIT:

	;Send SENSE DRIVE STATUS to latch disk change
	ld	(ix+WK_CMD),04h
	ld	(ix+WK_CMD+1),a
	push	bc
	ld	b,2
	call	FDC_WRITE_CMD
	pop	bc
	jr	c,DSTAT_UNK

	;Wait for FDC request, then read S1990
DSTAT_POLL:
	call	READ_MSR
	add	a,a
	jr	nc,DSTAT_POLL

	;Read disk change from S1990 register (in slot 3-2)
	push	bc
	push	de
	push	ix
	ld	hl,S1990
	ld	a,FDC_SLOT
	call	RDSLT
	pop	ix
	pop	de
	pop	bc
	and	b
	ld	b,a		;Save disk change result

	call	FDC_READ_RESULT

	;Bit is active low: 0=changed, non-zero=not changed
	ld	a,b
	or	a
	ld	b,1		;Not changed
	jr	nz,DSTAT_RET
	ld	b,2		;Changed
DSTAT_RET:
	pop	ix
	xor	a
	ret

DSTAT_UNK:
	pop	ix
	ld	b,3		;Unknown
	xor	a
	ret


;--- Get device availability

DO_DEVQ_GET_AVAILABILITY:
	call	CHECK_DEVICE
	ret	nz
	ld	b,1
	xor	a
	ret


;--- Get format choices

DO_DEVQ_GET_FORMAT_CHOICES:
	call	CHECK_DEVICE
	ret	nz

	ld	b,1		;Single side and double side, double density
	xor	a
	ret


;--- Do format

DO_DEVQ_DO_FORMAT:
	call	CHECK_DEVICE
	ret	nz

	;B = choice: 1=single side, 2=double side
	;C = device number (1-based)
	;HL = buffer address (usable as scratch)

	push	ix
	push	hl		;Save buffer address
	call	MY_GWORK

	ld	a,b
	cp	1
	jr	z,DOFMT_SS
	cp	2
	jr	z,DOFMT_DS
	pop	hl
	pop	ix
	ld	a,QUERY_NOT_IMPLEMENTED
	ret

DOFMT_SS:
	ld	a,1		;1 side
	jr	DOFMT_GO
DOFMT_DS:
	ld	a,2		;2 sides
DOFMT_GO:
	ld	(ix+FMT_SIDES),a
	ld	a,c
	dec	a		;0-based drive number
	ld	(ix+FMT_DRIVE),a
	ld	(ix+WK_CMD+1),a	;Set unit in FDC command byte

	;Turn on motor
	or	a
	ld	a,14h		;DOR: motor on, drive 0
	jr	z,DOFMT_DOR
	ld	a,25h		;DOR: motor on, drive 1
DOFMT_DOR:
	call	WRITE_DOR

	;Wait for drive ready
	ld	a,20h
	call	WRITE_TDR	;READY input from drive
	call	FDC_WAIT_READY
	push	af		;Save ST3 + carry
	ld	a,30h
	call	WRITE_TDR	;Force READY high
	pop	af
	jr	c,DOFMT_NRDY

	;Check write protect (ST3 bit 6)
	bit	6,a
	jr	nz,DOFMT_WP

	;Format each track (80 tracks)
	pop	hl		;HL = scratch buffer
	ld	a,0		;Start at track 0
DOFMT_TRK:
	push	af		;Save track number
	ld	(ix+FMT_TRACK),a
	ld	a,(ix+FMT_SIDES)
	ld	b,a		;B = number of sides
	ld	a,0		;Start at side 0
DOFMT_SIDE:
	push	bc
	push	af		;Save side number
	push	hl		;Save buffer
	ld	(ix+FMT_SIDE),a
	call	FORMAT_TRACK
	ld	d,a		;Save error code
	pop	hl
	pop	af
	pop	bc
	ld	a,d		;Restore error code
	or	a
	jr	nz,DOFMT_ERR
	inc	a		;Next side
	djnz	DOFMT_SIDE

	pop	af		;Restore track number
	inc	a
	cp	80		;80 tracks per side
	jr	c,DOFMT_TRK

	;Initialize boot sector, FAT, and root directory
	call	DOFMT_INIT
	push	af
	call	DOFMT_TIMER
	pop	af
	pop	ix
	ret			;A = error from DOFMT_INIT (0=success)

DOFMT_ERR:
	ld	b,a		;Save error code in B
	pop	af		;Discard saved track number
	ld	a,b		;Restore error code
	push	af		;Save error code on stack
	call	DOFMT_TIMER
	pop	af		;Restore error code
	pop	ix
	ret			;A = error code

DOFMT_NRDY:
	pop	hl		;Discard saved buffer
	call	DOFMT_TIMER
	pop	ix
	ld	a,_NRDY
	ret

DOFMT_WP:
	pop	hl		;Discard saved buffer
	call	DOFMT_TIMER
	pop	ix
	ld	a,_WPROT
	ret

;--- Initialize boot sector, FAT, and root directory after format.
;    Input: IX = work area, HL = 512-byte scratch buffer
;           FMT_DRIVE = 0-based drive, FMT_SIDES = 1 or 2
;    Output: A = error code (0=success), Carry set on error

DOFMT_INIT:

	;--- Determine media descriptor and sectors/FAT

	ld	a,(ix+FMT_SIDES)
	cp	2
	ld	c,0F9h		;DS/DD: media ID
	jr	z,DI_MEDOK
	ld	c,0F8h		;SS/DD: media ID
DI_MEDOK:
	ld	(ix+FMT_SIDE),c	;Reuse FMT_SIDE to store media ID

	;--- Clear buffer and build boot sector

	call	DI_CLEAR_BUF
	push	hl
	ex	de,hl		;DE = buffer
	ld	hl,BOOT_DATA
	ld	bc,BOOT_PARMS_OFF
	ldir			;Copy header up to disk parameters
	;Copy 720K parameters by default
	ld	hl,BOOT_PARMS_720K
	ld	a,(ix+FMT_SIDES)
	cp	2
	jr	z,DI_PARMS
	ld	hl,BOOT_PARMS_360K
DI_PARMS:
	ld	bc,BOOT_PARMS_LEN
	ldir			;Copy disk parameters
	;Copy rest of boot sector (code + messages)
	ld	hl,BOOT_DATA+BOOT_PARMS_OFF
	ld	bc,BOOT_TAIL_LEN
	ldir
	pop	hl		;HL = buffer

	;--- Write boot sector (sector 0)

	call	DI_WRITE_SECTORS_0
	ret	c

	;--- Build first FAT sector: media ID, FFh, FFh, rest zeros

	call	DI_CLEAR_BUF
	ld	a,(ix+FMT_SIDE)	;Media ID
	ld	(hl),a
	inc	hl
	ld	(hl),0FFh
	inc	hl
	ld	(hl),0FFh
	dec	hl
	dec	hl		;HL = buffer start

	;--- Determine sectors/FAT

	ld	a,(ix+FMT_SIDES)
	cp	2
	ld	b,3		;DS: 3 sectors/FAT
	jr	z,DI_FATCNT
	ld	b,2		;SS: 2 sectors/FAT
DI_FATCNT:
	ld	(ix+FMT_TRACK),b	;Reuse FMT_TRACK to store sects/FAT

	;--- Write first FAT sector (sector 1).

	;FMT_SECNUM is already 1 from boot sector write.
	ld	b,1
	call	DI_WRITE_SECTORS
	ret	c

	;--- Clear buffer, write remaining sectors of first FAT (zeros)

	call	DI_CLEAR_BUF
	ld	a,(ix+FMT_TRACK)	;sectors/FAT
	dec	a			;Already wrote 1
	jr	z,DI_FAT2		;Only 1 sector/FAT? Skip
	ld	b,a
	call	DI_WRITE_SECTORS
	ret	c

DI_FAT2:
	;--- Write second FAT copy: first sector has media ID + FF FF

	ld	a,(ix+FMT_SIDE)	;Media ID
	ld	(hl),a
	inc	hl
	ld	(hl),0FFh
	inc	hl
	ld	(hl),0FFh
	dec	hl
	dec	hl
	ld	b,1
	call	DI_WRITE_SECTORS
	ret	c

	;--- Clear buffer, write remaining sectors of second FAT (zeros)

	call	DI_CLEAR_BUF
	ld	a,(ix+FMT_TRACK)	;sectors/FAT
	dec	a
	jr	z,DI_ROOTDIR
	ld	b,a
	call	DI_WRITE_SECTORS
	ret	c

DI_ROOTDIR:
	;--- Buffer is already zeroed. Write 7 root directory sectors.

	ld	b,7
	jp	DI_WRITE_SECTORS

;--- Helper: write 1 sector at sector 0, buffer at HL

DI_WRITE_SECTORS_0:
	ld	(ix+FMT_SECNUM),0
	ld	(ix+FMT_SECNUM+1),0
	ld	b,1
	jr	DI_WRITE_SECTORS

;--- Helper: write B sectors from FMT_SECNUM, buffer at HL.
;    Writes 1 sector at a time (buffer is reused, not advanced).
;    Input: IX = work area, HL = buffer, B = sector count
;           FMT_SIDE = media descriptor, FMT_DRIVE = 0-based drive
;    Output: A = error, Carry set on error

DI_WRITE_SECTORS:
	push	hl
	push	bc
	;DE = address of FMT_SECNUM in work area (IX+FMT_SECNUM)
	push	ix
	pop	de
	inc	de		;+1
	inc	de		;+2
	inc	de		;+3 = FMT_SECNUM
	ld	b,1		;Write 1 sector
	ld	c,(ix+FMT_SIDE)	;C = media descriptor (stored earlier)
	ld	a,(ix+FMT_DRIVE)
	inc	a		;1-based device number
	scf			;Write operation
	call	READ_WRITE
	pop	bc
	pop	hl
	ret	c
	;Advance sector number
	inc	(ix+FMT_SECNUM)
	jr	nz,DI_WS_NOV
	inc	(ix+FMT_SECNUM+1)
DI_WS_NOV:
	djnz	DI_WRITE_SECTORS
	xor	a		;Success (carry clear)
	ret

;--- Helper: clear 512-byte buffer at HL (preserves HL)

DI_CLEAR_BUF:
	push	hl
	push	bc
	ld	d,h
	ld	e,l
	inc	de
	ld	(hl),0
	ld	bc,511
	ldir
	pop	bc
	pop	hl
	ret

;--- Boot sector template data

BOOT_DATA:
	db	0EBh,0FEh,090h
	db	"NEXTOR  "
	db	000h,002h
	;Disk parameters are inserted separately
	;Rest of boot sector (code + messages) after parameters
	db	000h,000h,000h,0D0h,0EDh
	db	053h,059h,0C0h,032h,0C4h,0C0h,036h,056h
	db	023h,036h,0C0h,031h,01Fh,0F5h,011h,09Fh
	db	0C0h,00Eh,00Fh,0CDh,07Dh,0F3h,03Ch,0CAh
	db	063h,0C0h,011h,000h,001h,00Eh,01Ah,0CDh
	db	07Dh,0F3h,021h,001h,000h,022h,0ADh,0C0h
	db	021h,000h,03Fh,011h,09Fh,0C0h,00Eh,027h
	db	0CDh,07Dh,0F3h,0C3h,000h,001h,058h,0C0h
	db	0CDh,000h,000h,079h,0E6h,0FEh,0FEh,002h
	db	0C2h,06Ah,0C0h,03Ah,0C4h,0C0h,0A7h,0CAh
	db	022h,040h,011h,079h,0C0h,00Eh,009h,0CDh
	db	07Dh,0F3h,00Eh,007h,0CDh,07Dh,0F3h,018h
	db	0B2h
	db	"Boot error",13,10
	db	"Press any key for retry",13,10,"$",0
	db	"MSXDOS  SYS"

BOOT_PARMS_OFF	equ	13	;Offset of disk parameters in boot sector
BOOT_TAIL_LEN	equ	$-BOOT_DATA-BOOT_PARMS_OFF

BOOT_PARMS_360K:
	db	002h,001h,000h,002h,070h,000h,0D0h,002h
	db	0F8h,002h,000h,009h,000h,001h

BOOT_PARMS_720K:
	db	002h,001h,000h,002h,070h,000h,0A0h,005h
	db	0F9h,003h,000h,009h,000h,002h
BOOT_PARMS_END:

BOOT_PARMS_LEN	equ	BOOT_PARMS_END-BOOT_PARMS_720K	;Length of disk parameters

DOFMT_TIMER:
	ld	a,(ix+FMT_DRIVE)
	or	a
	jr	nz,DOFMT_TM1
	ld	(ix+WK_MT0),MOTOR_TIMEOUT
	ret
DOFMT_TM1:
	ld	(ix+WK_MT1),MOTOR_TIMEOUT
	ret


;--- Format one track
;    Input: IX = work area (WK_CMD+1 = unit byte with HD and US set)
;           ix+FMT_TRACK = track, ix+FMT_SIDE = side
;           HL = scratch buffer (at least 36 bytes, in page 2/3)
;    Output: A = error code (0=success)

FORMAT_TRACK:
	push	hl		;Save buffer pointer

	;Set head select in unit byte
	ld	a,(ix+FMT_SIDE)
	or	a
	jr	z,FT_HD0
	set	2,(ix+WK_CMD+1)
	jr	FT_HDOK
FT_HD0:
	res	2,(ix+WK_CMD+1)
FT_HDOK:

	;Seek to the track
	push	hl
	push	de
	ld	c,(ix+FMT_TRACK)
	call	FDC_SEEK
	pop	de
	pop	hl
	jr	c,FT_ERR_SEEK

	;Build format ID data in buffer: 9 sectors * 4 bytes (C,H,R,N)
	pop	hl		;HL = scratch buffer
	push	hl		;Save it again for later
	ld	b,9		;9 sectors per track
	ld	c,1		;First sector number
FT_BUILD:
	ld	a,(ix+FMT_TRACK)
	ld	(hl),a		;C = cylinder
	inc	hl
	ld	a,(ix+FMT_SIDE)
	ld	(hl),a		;H = head
	inc	hl
	ld	a,c
	ld	(hl),a		;R = sector number
	inc	hl
	ld	(hl),2		;N = size code (2=512 bytes)
	inc	hl
	inc	c
	djnz	FT_BUILD

	;Build FDC FORMAT TRACK command in WK_CMD
	ld	(ix+WK_CMD),4Dh	;FORMAT TRACK (MFM)
	;WK_CMD+1 already has HD|US
	ld	(ix+WK_CMD+2),2	;N = 2 (512 bytes/sector)
	ld	(ix+WK_CMD+3),9	;SC = 9 sectors per track
	ld	(ix+WK_CMD+4),50h	;GPL = format gap length
	ld	(ix+WK_CMD+5),0E5h	;D = fill byte

	;Send command to FDC (6 bytes)
	ld	b,6
	call	FDC_WRITE_CMD
	jr	c,FT_ERR_CMD

	;Send 36 bytes of ID data via fast RAM transfer routine.
	;CALL_XFER switches page 1 to FDC slot for direct register
	;access, meeting FDC timing requirements. The XFER loop exits
	;when EXM clears (format complete), then sends TC pulse.
	pop	hl		;HL = buffer with ID data
	push	hl
	set	0,(ix+WK_FLAGS)	;Write mode (CPU->FDC)
	call	CALL_XFER

	;Read result bytes
	call	FDC_READ_RESULT

	;Check ST0 for errors
	ld	a,(ix+WK_RES)	;ST0
	and	0C0h
	jr	nz,FT_ERR_FDC

	pop	hl		;Restore buffer pointer
	xor	a		;Success
	ret

FT_ERR_SEEK:
	pop	hl
	ld	a,_SEEK
	ret

FT_ERR_CMD:
	pop	hl
	ld	a,_DISK
	ret

FT_ERR_FDC:
	pop	hl
	call	ERROR_FROM_ST	;A = error code from ST1/ST2
	ret


;=============================================================================
;  READ / WRITE
;=============================================================================

    ;    Input:    Cy=0 to read, 1 to write
    ;              A = Device number (1-based)
    ;              B = Number of sectors to read or write
    ;              C = Media descriptor byte (F8h-FFh, or 0 if unknown)
    ;              HL = Source or destination memory address
    ;              DE = Address where the 4 byte sector number is stored
    ;    Output:   A = Error code (0 = OK)

READ_WRITE:
	;--- Save carry (read/write) and device number

	push	af		;[SP+6] = device + flags
	push	bc		;[SP+4] = B=sectors, C=media
	push	de		;[SP+2] = sector number pointer
	push	hl		;[SP+0] = buffer

	call	MY_GWORK	;IX = work area, all regs preserved

	pop	hl		;HL = buffer
	pop	de		;DE = sector number pointer
	pop	bc		;B = sectors, C = media
	pop	af		;A = device, Cy = read/write

	;--- Save write flag in work area

	res	0,(ix+WK_FLAGS)
	jr	nc,RW_NOWR
	set	0,(ix+WK_FLAGS)
RW_NOWR:

	;--- Validate device number

	push	bc		;Save sectors + media
	ld	b,a		;B = device number
	or	a
	jr	z,RW_BADDEV
	ld	a,(ix+WK_NDRV)
	cp	b		;ndrv < device?
	jr	c,RW_BADDEV	;Yes, invalid

	;--- Store 0-based unit in FDC command byte 1

	ld	a,b
	dec	a		;0-based unit
	ld	(ix+WK_CMD+1),a
	pop	bc		;Restore B=sectors, C=media
	jr	RW_DEVOK

RW_BADDEV:
	pop	bc
	ld	a,_IDEVN
	ret

RW_DEVOK:

	;--- Validate/default media descriptor

	ld	a,c
	cp	0F8h
	jr	nc,RW_MEDOK
	ld	c,0F9h		;Default: 720KB (9spt, 2 sides)
RW_MEDOK:

	;--- Read 16-bit sector number from (DE)

	push	hl		;Save buffer
	ex	de,hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)		;DE = sector number (lower 16 bits)
	pop	hl		;HL = buffer

	;--- Set constant FDC command parameters

	ld	(ix+WK_CMD+5),2	;N = 512 bytes
	ld	(ix+WK_CMD+7),80	;GPL
	ld	(ix+WK_CMD+8),0FFh	;DTL

	;--- Determine sectors per track

	bit	1,c		;Media bit 1: 1=8spt, 0=9spt
	ld	a,9
	jr	z,RW_9SPT
	ld	a,8
RW_9SPT:
	ld	(ix+WK_CMD+6),a	;EOT

	;--- Divide sector number by sectors-per-track
	;    DE = sector number, A = spt
	;    After: track in L, sector-in-track (0-based) in A

	push	bc		;Save B=count, C=media
	push	hl		;Save buffer
	ex	de,hl		;HL = sector number
	ld	e,a		;E = spt
	call	DIV_HL_E	;HL = quotient (track), A = remainder
	inc	a		;1-based sector
	ld	(ix+WK_CMD+4),a	;R = sector
	ld	a,l		;A = track number
	pop	hl		;HL = buffer
	pop	bc		;B = count, C = media

	;--- Determine cylinder and head from track
	;    A = track, C = media descriptor
	;    For DS (bit 0 of C set): cylinder = track/2, head = track%2
	;    For SS: cylinder = track, head = 0

	bit	0,c		;Double-sided?
	jr	z,RW_SS

	;Double-sided
	srl	a		;A = cylinder, Cy = head
	jr	nc,RW_HD0
	set	2,(ix+WK_CMD+1)	;Head 1 in unit byte
	ld	(ix+WK_CMD+3),1	;H = 1
	jr	RW_HDOK

RW_SS:	;Single-sided, fall through to head 0
RW_HD0:
	res	2,(ix+WK_CMD+1)
	ld	(ix+WK_CMD+3),0	;H = 0

RW_HDOK:
	ld	(ix+WK_CMD+2),a	;Cylinder

	;--- Build D register: media info (bits 7-6) + DOR (bits 5-0)
	;    This is carried through the transfer loop for NEXT_SECTOR

	push	af		;Save cylinder (for seek)
	ld	a,(ix+WK_CMD+1)
	and	1		;Unit (0 or 1)
	or	a
	ld	a,14h		;DOR for drive 0
	jr	z,RW_DOR0
	ld	a,25h		;DOR for drive 1
RW_DOR0:
	ld	d,a		;D = DOR temporarily
	ld	a,c		;Media descriptor
	rrca
	rrca
	and	0C0h		;Media bits 1,0 -> bits 7,6
	or	d		;Combine with DOR
	ld	d,a		;D = media info + DOR
	pop	af		;A = cylinder

	;--- Turn on motor

	push	af		;Save cylinder
	ld	a,d
	and	3Fh		;Extract DOR value
	call	WRITE_DOR
	pop	af		;A = cylinder
	ld	c,a		;C = cylinder (for seek)

	;--- Wait for drive ready

	push	af
	ld	a,20h
	call	WRITE_TDR	;READY input from drive
	pop	af

	push	hl
	push	bc
	push	de
	call	FDC_WAIT_READY	;Returns A = ST3 on success
	push	af		;Save ST3 + carry
	ld	a,30h
	call	WRITE_TDR	;READY input true
	pop	af
	pop	de
	pop	bc
	pop	hl
	jr	c,RW_ERR_NRDY

	;--- Check write protect (ST3 bit 6)

	bit	6,a
	jr	z,RW_NOWP
	bit	0,(ix+WK_FLAGS)
	jr	nz,RW_ERR_WP
RW_NOWP:

	;--- Seek to cylinder (C = cylinder)

	push	hl
	push	bc
	push	de

	;If track 0, recalibrate first for reliability
	ld	a,c
	or	a
	jr	nz,RW_SKN0
	push	bc
	ld	c,6
	call	FDC_SEEK
	call	FDC_RECALIBRATE
	pop	bc
	ld	c,0
RW_SKN0:
	call	FDC_SEEK

	pop	de
	pop	bc
	pop	hl
	jr	c,RW_ERR_SEEK

	;--- Transfer loop: read/write B sectors
	;    B = sector count, D = media+DOR, HL = buffer, IX = work area

RW_LOOP:
	push	hl		;Save buffer for this sector
	push	bc
	push	de

	push	af
	ld	a,20h
	call	WRITE_TDR	;READY from drive
	pop	af

	ld	e,11		;Retry counter

RW_RETRY:
	bit	0,(ix+WK_FLAGS)
	jr	nz,RW_DOWR

	call	READ_SECTOR
	jr	RW_CHKRES

RW_DOWR:
	call	WRITE_SECTOR

RW_CHKRES:
	push	af
	ld	a,30h
	call	WRITE_TDR	;READY true
	pop	af

	ld	a,(ix+WK_RES)	;ST0
	and	0C8h
	jr	z,RW_SECTOK

	;Error: check if not-ready
	and	08h
	jr	nz,RW_NRDY_LP

	;Reposition and retry
	call	REPOSITION
	dec	e
	jr	nz,RW_RETRY

	;Retries exhausted
	call	ERROR_FROM_ST
	pop	de
	pop	bc
	pop	hl
	jr	RW_TIMER

RW_SECTOK:
	pop	de
	pop	bc
	pop	hl

	;Advance buffer by 512
	inc	h
	inc	h

	dec	b
	jr	z,RW_DONE

	;Advance to next sector (may seek)
	call	NEXT_SECTOR
	jr	RW_LOOP

RW_DONE:
	xor	a		;Success

RW_TIMER:
	;--- Set motor timeout timer and return
	push	af
	ld	a,d
	bit	0,a		;Drive select bit
	jr	nz,RW_TM1
	ld	(ix+WK_MT0),MOTOR_TIMEOUT
	jr	RW_TMOK
RW_TM1:
	ld	(ix+WK_MT1),MOTOR_TIMEOUT
RW_TMOK:
	pop	af
	ret

RW_ERR_NRDY:
	ld	a,_NRDY
	jr	RW_TIMER

RW_ERR_WP:
	ld	a,_WPROT
	jr	RW_TIMER

RW_ERR_SEEK:
	ld	a,_SEEK
	jr	RW_TIMER

RW_NRDY_LP:
	pop	de
	pop	bc
	pop	hl
	ld	a,_NRDY
	jr	RW_TIMER


;=============================================================================
;  FDC LOW-LEVEL ROUTINES (using inter-slot RDSLT/WRSLT)
;=============================================================================

;--- Initialize FDC
;    Input: IX = work area

FDC_INIT:
	xor	a
	call	WRITE_DOR	;Reset FDC (bit 2 = 0)
	ld	a,0FAh		;Force READY high, enable TC
	call	WRITE_TDR
	ld	a,04h
	call	WRITE_DOR	;FDC enabled, all motors off

	;Send SPECIFY command: SRT=3ms, HUT=240ms, HLT=2ms, Non-DMA
	ld	(ix+WK_CMD),03h
	ld	(ix+WK_CMD+1),0DFh
	ld	(ix+WK_CMD+2),03h
	ld	b,3
	jp	FDC_WRITE_CMD


;--- Turn motor off

MOTOR_OFF:
	ld	a,04h		;FDC enabled, all motors off
	jp	WRITE_DOR


;--- Write B command bytes from WK_CMD to FDC
;    Input: IX = work area, B = byte count
;    Output: Cy=0 OK, Cy=1 timeout
;    Preserves: HL

FDC_WRITE_CMD:
	push	hl
	ld	hl,07D0h	;Timeout counter

FWC_BUSY:
	push	hl
	call	READ_MSR
	pop	hl
	and	10h		;FDC busy?
	jr	z,FWC_SEND
	dec	hl
	ld	a,h
	or	l
	jr	nz,FWC_BUSY
	pop	hl
	scf
	ret

FWC_SEND:
	;Get pointer to command buffer into DE
	push	ix
	pop	de
	push	hl		;Save timeout HL (will be popped at end)
	ld	hl,WK_CMD
	add	hl,de
	ex	de,hl		;DE = &WK_CMD

	;Send bytes one by one
FWC_LP:
	call	READ_MSR
	and	0C0h
	cp	80h
	jr	nz,FWC_LP	;Wait for output request
	ld	a,(de)		;Get command byte from buffer
	inc	de
	call	WRITE_DAT	;Send to FDC via inter-slot write
	djnz	FWC_LP

	xor	a		;Clear carry
	pop	hl
	pop	hl		;Restore caller's HL
	ret


;--- Read FDC result bytes into WK_RES
;    Input: IX = work area
;    Preserves: HL

FDC_READ_RESULT:
	push	hl
	push	ix
	pop	de
	ld	hl,WK_RES
	add	hl,de
	ex	de,hl		;DE = &WK_RES

FRR_LP:
	call	READ_MSR
	add	a,a		;Bit 7 (RQM) -> Carry
	jr	nc,FRR_LP	;Wait for RQM
	bit	7,a		;After shift: bit 7 = orig bit 6 (DIO)
	jr	z,FRR_DN	;If DIO=0, result phase done
	call	READ_DAT
	ld	(de),a
	inc	de
	jr	FRR_LP

FRR_DN:
	pop	hl
	ret


;--- Wait for seek to complete
;    Output: Cy=0 OK, Cy=1 error

FDC_WAIT_SEEK:
	ld	hl,07D0h	;Timeout counter
FWS_BSY:
	call	READ_MSR
	and	10h
	jr	z,FWS_CHK_INIT	;Not busy: proceed to sense phase
	dec	hl
	ld	a,h
	or	l
	jr	nz,FWS_BSY
	scf			;Timeout
	ret

FWS_CHK_INIT:
	ld	hl,07D0h	;Timeout counter
FWS_CHK:
	call	FDC_SENSE_INT
	jr	c,FWS_TOUT	;FDC_SENSE_INT timed out
	ld	a,(ix+WK_RES)	;ST0
	bit	5,a		;Seek complete?
	jr	nz,FWS_DONE
	dec	hl
	ld	a,h
	or	l
	jr	nz,FWS_CHK
FWS_TOUT:
	scf			;Timeout
	ret

FWS_DONE:
	and	0C0h		;Check for errors
	ret	z		;Normal termination (Cy=0)
	scf
	ret


;--- Sense Interrupt Status
;    Output: Cy=0 OK, Cy=1 timeout (FDC_WRITE_CMD failed)

FDC_SENSE_INT:
	push	bc
	ld	(ix+WK_CMD),08h
	ld	b,1
	call	FDC_WRITE_CMD
	jr	c,FSI_END	;Timeout: skip read, return with Cy
	call	FDC_READ_RESULT
	or	a		;Clear carry (success)
FSI_END:
	pop	bc
	ret


;--- Seek to track
;    Input: IX = work area, C = track number
;    Output: Cy=0 OK, Cy=1 error
;    Preserves: BC, DE, HL

FDC_SEEK:
	push	bc

	;Pre-seek delay
	ld	b,77h
FDS_DL1:
	ex	(sp),hl
	ex	(sp),hl
	djnz	FDS_DL1

	ld	(ix+WK_CMD),0Fh	;SEEK
	ld	(ix+WK_CMD+2),c	;Track
	ld	b,3
	call	FDC_WRITE_CMD
	jr	c,FDS_END
	call	FDC_WAIT_SEEK
FDS_END:

	;Post-seek settle delay
	push	af
	ld	bc,0773h
FDS_DL2:
	dec	bc
	ld	a,b
	or	c
	jr	nz,FDS_DL2
	pop	af

	pop	bc
	ret


;--- Recalibrate (seek to track 0)
;    Output: Cy=0 OK, Cy=1 error

FDC_RECALIBRATE:
	ld	(ix+WK_CMD),07h
	ld	b,2
	call	FDC_WRITE_CMD
	ret	c
	jp	FDC_WAIT_SEEK


;--- Wait for drive ready using SENSE DRIVE STATUS
;    Input: IX = work area (WK_CMD+1 must have unit byte)
;    Output: Cy=0 ready (A=ST3), Cy=1 timeout

FDC_WAIT_READY:
	ld	(ix+WK_CMD),04h
	push	bc
	push	hl
	ld	hl,1388h

FWR_LP:
	dec	hl
	ld	a,l
	or	h
	jr	z,FWR_TO

	ld	b,2
	call	FDC_WRITE_CMD
	call	FDC_READ_RESULT
	ld	a,(ix+WK_RES)	;ST3
	bit	5,a		;Ready?
	jr	z,FWR_LP

	pop	hl
	pop	bc
	or	a		;Clear carry
	ret

FWR_TO:
	pop	hl
	pop	bc
	scf
	ret


;--- Read one sector from FDC
;    Input: IX = work area, HL = destination buffer
;    FDC command fields must be set (unit, C, H, R, N, EOT, GPL, DTL)
;    Output: result in WK_RES

READ_SECTOR:
	push	hl
	push	de
	push	bc

	ld	(ix+WK_CMD),46h	;READ DATA, MFM
	ld	b,9
	call	FDC_WRITE_CMD
	jr	c,RS_CMDER

	;Data transfer via RAM routine (switches page 1 to slot 3-2)
	call	CALL_XFER

	;Read result phase via RDSLT (not time-critical)
	call	FDC_READ_RESULT

	pop	bc
	pop	de
	pop	hl
	ret

RS_CMDER:
	ld	(ix+WK_RES),0C8h	;Fake error in ST0
	pop	bc
	pop	de
	pop	hl
	ret


;--- Write one sector to FDC
;    Input: IX = work area, HL = source buffer
;    Output: result in WK_RES

WRITE_SECTOR:
	push	hl
	push	de
	push	bc

	ld	(ix+WK_CMD),45h	;WRITE DATA, MFM
	ld	b,9
	call	FDC_WRITE_CMD
	jr	c,RS_CMDER

	;Data transfer via RAM routine
	call	CALL_XFER

	;Read result phase
	call	FDC_READ_RESULT

	pop	bc
	pop	de
	pop	hl
	ret


;--- Advance to next sector
;    Input: D = media info + DOR, IX = work area
;    Updates WK_CMD fields, may seek to next cylinder

NEXT_SECTOR:
	ld	a,(ix+WK_CMD+4)	;R (current sector)
	inc	a
	ld	(ix+WK_CMD+4),a

	;Check if past end of track
	bit	7,d		;8 spt? (media bit 1)
	jr	nz,NS_8SPT
	cp	10		;Past sector 9?
	ret	c
	jr	NS_NXTTRK
NS_8SPT:
	cp	9		;Past sector 8?
	ret	c

NS_NXTTRK:
	ld	(ix+WK_CMD+4),1	;Back to sector 1

	;Check double-sided
	bit	6,d		;DS? (media bit 0)
	jr	z,NS_NXTCYL

	;Toggle head
	ld	a,(ix+WK_CMD+3)
	xor	1
	ld	(ix+WK_CMD+3),a
	jr	z,NS_NXTCYL	;Was head 1, now 0: next cylinder
	set	2,(ix+WK_CMD+1);Head 1 in unit byte
	ret

NS_NXTCYL:
	res	2,(ix+WK_CMD+1);Head 0 in unit byte
	ld	c,(ix+WK_CMD+2);Current cylinder
	inc	c
	ld	(ix+WK_CMD+2),c
	jp	FDC_SEEK


;--- Reposition for error recovery: seek to track 6, recalibrate, seek back
;    Input: E = retry counter (only reposition on even retries)

REPOSITION:
	bit	0,e
	ret	nz		;Skip on odd retries

	push	bc
	push	de
	ld	c,6
	call	FDC_SEEK
	call	FDC_RECALIBRATE
	ld	c,(ix+WK_CMD+2)	;Original cylinder
	call	FDC_SEEK
	pop	de
	pop	bc
	ret


;--- Determine error code from FDC status registers
;    Input: IX = work area (WK_RES filled)
;    Output: A = error code, Cy set

ERROR_FROM_ST:
	ld	a,(ix+WK_RES+1)	;ST1
	bit	2,a
	ld	a,_RNF
	scf
	ret	nz
	ld	a,(ix+WK_RES+1)
	bit	5,a
	ld	a,_DATA
	scf
	ret	nz
	ld	a,(ix+WK_RES+1)
	bit	1,a
	ld	a,_WPROT
	scf
	ret	nz
	ld	a,_DISK
	scf
	ret


;=============================================================================
;  INTER-SLOT FDC ACCESS WRAPPERS
;=============================================================================

;--- Read MSR register via RDSLT
;    Output: A = MSR value
;    Preserves: BC, DE, HL, IX

READ_MSR:
	push	bc
	push	de
	push	hl
	push	ix
	ld	hl,FDC_MSR
	ld	a,FDC_SLOT
	call	RDSLT
	pop	ix
	pop	hl
	pop	de
	pop	bc
	ret


;--- Read DAT register via RDSLT
;    Output: A = data byte
;    Preserves: BC, DE, HL, IX

READ_DAT:
	push	bc
	push	de
	push	hl
	push	ix
	ld	hl,FDC_DAT
	ld	a,FDC_SLOT
	call	RDSLT
	pop	ix
	pop	hl
	pop	de
	pop	bc
	ret


;--- Write to DOR register via WRSLT
;    Input: A = value
;    Preserves: BC, DE, HL, IX

WRITE_DOR:
	push	bc
	push	de
	push	hl
	push	ix
	ld	hl,FDC_DOR
	ld	e,a
	ld	a,FDC_SLOT
	call	WRSLT
	pop	ix
	pop	hl
	pop	de
	pop	bc
	ret


;--- Write to TDR register via WRSLT
;    Input: A = value
;    Preserves: BC, DE, HL, IX

WRITE_TDR:
	push	bc
	push	de
	push	hl
	push	ix
	ld	hl,FDC_TDR
	ld	e,a
	ld	a,FDC_SLOT
	call	WRSLT
	pop	ix
	pop	hl
	pop	de
	pop	bc
	ret


;--- Write to DAT register via WRSLT
;    Input: A = value
;    Preserves: BC, DE, HL, IX

WRITE_DAT:
	push	bc
	push	de
	push	hl
	push	ix
	ld	hl,FDC_DAT
	ld	e,a
	ld	a,FDC_SLOT
	call	WRSLT
	pop	ix
	pop	hl
	pop	de
	pop	bc
	ret


;=============================================================================
;  RAM TRANSFER ROUTINE TRAMPOLINE
;=============================================================================

;--- Call the RAM transfer routine in the work area
;    Input: IX = work area, HL = buffer
;           WK_FLAGS bit 0 = 0 for read, 1 for write
;    Trashes: IY, DE, BC
;    Note: This is a trampoline; the RAM routine's RET returns to our caller.

CALL_XFER:
	;--- Patch the transfer direction bytes in the RAM routine
	bit	0,(ix+WK_FLAGS)
	jr	nz,XFER_WR_P
	ld	(ix+WK_XFER+XFER_PATCH_OFF),1Ah	;ld a,(de)
	ld	(ix+WK_XFER+XFER_PATCH_OFF+1),77h	;ld (hl),a
	jr	XFER_JP
XFER_WR_P:
	ld	(ix+WK_XFER+XFER_PATCH_OFF),7Eh	;ld a,(hl)
	ld	(ix+WK_XFER+XFER_PATCH_OFF+1),12h	;ld (de),a
XFER_JP:
	;--- Compute RAM routine address (B=0 from high byte of WK_XFER)
	ex	de,hl		;DE = buffer
	push	ix
	pop	hl
	ld	bc,WK_XFER
	add	hl,bc		;HL = address of RAM routine
	push	hl
	pop	iy		;IY = RAM routine address

	;--- Precalculate slot switch/restore values and push for XFER_CODE
	;    Stack order (top=first popped): new_A8, new_FFFF, old_FFFF, old_A8
	;    B stays 0 throughout (only C is used as scratch)
	in	a,(0A8h)
	push	af		;old A8h (popped last: restore primary)
	and	11110011b
	or	00001100b	;Primary slot 3 for page 1
	ld	c,a		;C = new A8h value

	ld	a,(0FFFFh)
	cpl
	push	af		;old FFFFh (popped 3rd: restore secondary)
	and	11110011b
	or	00001000b	;Secondary slot 2 for page 1
	push	af		;new FFFFh (popped 2nd: switch secondary)

	ld	a,c
	push	af		;new A8h (popped 1st: switch primary)

	;--- Jump to RAM routine
	ex	de,hl		;HL = buffer
	jp	(iy)


;=============================================================================
;  RAM TRANSFER ROUTINE (position-independent, copied to work area at init)
;=============================================================================
;
; This code runs from page 3 RAM while page 1 is temporarily switched to
; slot 3-2 for direct FDC register access. Interrupts are disabled during
; the transfer to prevent the timer interrupt from accessing page 1.
;
; Entry: HL = buffer address (must be in page 2 or 3)
;        B = 0 (from CALL_XFER's ld bc,WK_XFER)
;        Stack (top to bottom): new_A8, new_FFFF, old_FFFF, old_A8, ret_addr
;        Two bytes at XFER_PATCH_OFF already patched by CALL_XFER:
;          Read:  1Ah 77h  (ld a,(de); ld (hl),a)
;          Write: 7Eh 12h  (ld a,(hl); ld (de),a)
; Exit:  Returns normally (page 1 restored, interrupts re-enabled)
;        FDC result bytes are NOT read here (caller must do FDC_READ_RESULT)

XFER_CODE_START:

	;--- Switch page 1 to slot 3-2 (values precalculated by CALL_XFER)
	di
	pop	af
	out	(0A8h),a		;Switch primary slot
	pop	af
	ld	(0FFFFh),a		;Switch secondary slot

	;--- Page 1 is now slot 3-2, FDC registers accessible
	;    Unified transfer loop: DE toggles between FDC_MSR and FDC_DAT
	;    B=0 on entry, so DJNZ loops 256 times per pass.
	;    C counts passes: 2 passes x 256 bytes = 512 bytes = 1 sector.

	ld	de,FDC_MSR
	ld	c,1
XFER_LP:
	ld	a,(de)			;Read MSR
	add	a,a			;RQM -> Carry
	jr	nc,XFER_LP		;Wait for RQM
	bit	6,a			;EXM (bit 5 of original -> bit 6 after shift)
	jr	z,XFER_TC		;EXM=0: execution phase done
	inc	de			;DE = FDC_DAT
XFER_PATCH:
	nop				;Patched by CALL_XFER (read or write)
	nop				;Patched by CALL_XFER (read or write)
	dec	de			;DE = FDC_MSR
	inc	hl
	djnz	XFER_LP
	dec	c
	jr	z,XFER_LP		;Second pass (C: 1->0, Z set)

	;=== TC PULSE ===
XFER_TC:
	ld	hl,FDC_TDR
	ld	(hl),02h		;Enable TC, TC=0
	ld	(hl),03h		;Enable TC, TC=1
	ld	(hl),02h		;Enable TC, TC=0

	;=== RESTORE SLOT STATE (values precalculated by CALL_XFER) ===
	pop	af
	ld	(0FFFFh),a		;Restore secondary slot
	pop	af
	out	(0A8h),a		;Restore primary slot

	ei
	ret

XFER_CODE_END:
XFER_CODE_SIZE:	equ XFER_CODE_END - XFER_CODE_START
XFER_PATCH_OFF:	equ XFER_PATCH - XFER_CODE_START


;=============================================================================
;  HELPER ROUTINES
;=============================================================================

;--- Get work area pointer
;    Output: IX = work area
;    Preserves: AF, BC, DE, HL

MY_GWORK:
	push	af
	push	bc
	push	de
	push	hl
	xor	a
	ex	af,af'
	xor	a
	ld	ix,GWORK
	call	CALBNK
	ld	l,(ix)
	ld	h,(ix+1)
	push	hl
	pop	ix
	pop	hl
	pop	de
	pop	bc
	pop	af
	ret


;--- Check device number validity
;    Input: C = device number
;    Output: A=0 and Z set if valid, A=QUERY_INVALID_DEVICE and NZ if invalid
;    Preserves: BC, DE, HL

CHECK_DEVICE:
	push	bc
	push	de
	push	hl
	call	MY_GWORK
	ld	a,c
	or	a
	jr	z,CD_BAD
	ld	b,(ix+WK_NDRV)
	cp	b
	jr	z,CD_GOOD
	jr	nc,CD_BAD
CD_GOOD:
	pop	hl
	pop	de
	pop	bc
	xor	a
	ret
CD_BAD:
	pop	hl
	pop	de
	pop	bc
	ld	a,QUERY_INVALID_DEVICE
	or	a		;Set NZ
	ret


;--- 16-bit division: HL / E
;    Input: HL = dividend, E = divisor (1-255)
;    Output: HL = quotient, A = remainder

DIV_HL_E:
	ld	b,16
	xor	a
DIV_LP:
	add	hl,hl
	rla
	cp	e
	jr	c,DIV_NS
	sub	e
	inc	l
DIV_NS:
	djnz	DIV_LP
	ret


;--- Copy string with length limit
;    Input: HL = source (zero-terminated), DE = dest, B = max length
;    Output: A = QUERY_OK or QUERY_TRUNCATED_STRING

OUTPUT_STRING:
	ld	a,b
	or	a
	ret	z

OS_LP:
	ld	a,(hl)
	or	a
	ld	(de),a
	ret	z		;A=0=QUERY_OK

	inc	hl
	inc	de
	djnz	OS_LP

	dec	de
	xor	a
	ld	(de),a
	ld	a,QUERY_TRUNCATED_STRING
	ret


;--- Print zero-terminated string via character output routine
;    Input: HL = string, DE = character output address (e.g. CHPUT)
;    Trashes: AF, HL, IX

PRINT_WITH_DE:
	push	de
	ld	de,0C300h
	push	de
	ld	ix,1
	add	ix,sp		;IX -> JP <charout> trampoline on stack
	call	PRINT_HL
	pop	de
	pop	de
	ret


;--- Print zero-terminated string
;    IX must point to a JP <charout> trampoline

PRINT_HL:
	ld	a,(hl)
	or	a
	ret	z
	call	JP_IX
	inc	hl
	jr	PRINT_HL

JP_IX:	jp	(ix)


;=============================================================================
;  DATA
;=============================================================================

	.stresc	on

STR_DRV_NAME:	db	"MSX Turbo-R FDD driver",0
STR_DRV_AUTHOR:	db	"Konamiman",0
STR_HW_NAME:	db	"TC8566AF FDC",0
STR_HW_AUTHOR:	db	"Panasonic",0
STR_DEV_NAME:	db	"Floppy disk drive",0

INIT_MSG:	db	"\r\nMSX Turbo-R FDD driver\r\n"
		db	"by Konamiman\r\n",0

STR_NDRV_1:	db	"\r\nFound ",0
STR_NDRV_2:	db	" drive(s)\r\n",0

STR_ERR_FDC:	db	"FDC not responding\r\n",0
STR_ERR_NODRIVE: db	"No drives found\r\n",0


;=============================================================================

	ds	7ED0h-$,0FFh

	end
