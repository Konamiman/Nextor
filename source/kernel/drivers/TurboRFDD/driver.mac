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
WK_FLAGS:	equ	5		;bit 0 = write operation
WK_CMD:	equ	10		;FDC command buffer (9 bytes, +10..+18)
WK_RES:	equ	19		;FDC result buffer (7 bytes, +19..+25)
WK_XFER:	equ	26		;Start of RAM transfer routine code

WK_XFER_SIZE:	equ 52		;Space reserved for RAM transfer routine
WK_SIZE:	equ	WK_XFER+WK_XFER_SIZE

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
	pop	de		;Discard saved CHPUT
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
	ld	a,QUERY_NOT_IMPLEMENTED
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
FWS_BSY:
	call	READ_MSR
	and	10h
	jr	nz,FWS_BSY	;Wait while FDC busy

FWS_CHK:
	call	FDC_SENSE_INT
	ld	a,(ix+WK_RES)	;ST0
	bit	5,a		;Seek complete?
	jr	z,FWS_CHK
	and	0C0h		;Check for errors
	ret	z		;Normal termination (Cy=0)
	scf
	ret


;--- Sense Interrupt Status

FDC_SENSE_INT:
	push	bc
	ld	(ix+WK_CMD),08h
	ld	b,1
	call	FDC_WRITE_CMD
	call	FDC_READ_RESULT
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
	call	FDC_WAIT_SEEK

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

STR_ERR_FDC:	db	"FDC not responding\r\n",0
STR_ERR_NODRIVE: db	"No drives found\r\n",0


;=============================================================================

	ds	7ED0h-$,0FFh

	end
