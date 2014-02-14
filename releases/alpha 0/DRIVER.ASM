	; Dummy disk driver for MSX-DOS 2.50.
	;
	; This code can be used as the basis for developing
	; a real disk driver, just set DRV_TYPE and DRV_HOTPLUG,
        ; change the driver name at DRV_NAME,
        ; and implement the required routines.
	;
	; See the driver development guide for more details.

	;org	4100h

DRV_START:

;-----------------------------------------------------------------------------
;
; Driver configuration constants
;

;Driver type:
;   0 for drive-based
;   1 for device-based

DRV_TYPE	equ	0

;Hot-plug devices support (device-based drivers only):
;   0 for no hot-plug support
;   1 for hot-plug support

DRV_HOTPLUG	equ	0

;Driver version

VER_MAIN	equ	1
VER_SEC		equ	0
VER_REV		equ	0

;-----------------------------------------------------------------------------
;
; Error codes for DEV_RW and DEV_FORMAT
;

if DRV_TYPE eq 1

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
.IDEVL:	equ	0B5h
.IPARM	equ	08Bh

endif

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
; Driver signature
;
	db	"MSXDOS_DRIVER",0

; Driver flags:
;    bit 0: 0 for drive-based, 1 for device-based
;    bit 1: 1 for hot-plug devices supported (device-based drivers only)

if DRV_TYPE eq 0
	db	0
endif

if DRV_TYPE eq 1
	db 1+(2*DRV_HOTPLUG)
endif

;Reserved byte
	db	0

;Driver name

DRV_NAME:
	db	"Dummy driver"
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

if DRV_TYPE eq 0
        jp      DRV_DSKIO
        jp      DRV_DSKCHG
        jp      DRV_GETDPB
        jp      DRV_CHOICE
        jp      DRV_DSKFMT
        jp      DRV_MTOFF
endif

if DRV_TYPE eq 1
	jp	DEV_RW
	jp	DEV_INFO
	jp	DEV_STATUS
	jp	LUN_INFO
	jp	DEV_FORMAT
	jp	DEV_CMD
endif


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
	xor	a
	ld	hl,0
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
;=====  BEGIN of DRIVE-BASED specific routines
;=====

if DRV_TYPE eq 0

;-----------------------------------------------------------------------------
;
; Read/write disk sectors
;
;Input:    A  = Drive number, starting at 0
;          Cy = 0 for reading sectors, 1 for writing sectors
;          B  = Number of sectors to read/write
;          DE = First sector number to read/write
;          HL = source/destination address for the transfer
;Output:   Cy = 0 on success, 1 on error
;          A  = Error code (on error only):
;               0   Write protected
;               2   Not ready
;               4   Data (CRC) error
;               6   Seek error
;               8   Record not found
;               10  Write fault
;               12  Other errors
          B = Number of sectors actually read (in case of error only)

DRV_DSKIO:
	ld	a,12
	scf
	ret


;-----------------------------------------------------------------------------
;
; Get disk change status
;
;Input:    A  = Drive number, starting at 0
;          B  = C = Media descriptor
;          HL = Base address for DPB -1
;Output:   Cy = 0 on success, 1 on error
;          A  = Error code (on error only)
;               Same codes as DRV_DSKIO
;          B  = Disk status (on success only)
;               1  Disk not changed
;               0  Unknown
;              -1  Disk changed

DRV_DSKCHG:
	ld	a,12
	scf
      	ret
      
      
;-----------------------------------------------------------------------------
;
; Get DPB for disk
;
;Input:    A  = Drive number, starting at 0
;          B  = C = Media descriptor
;          HL = Base address for DPB -1
;Output:   -

DRV_GETDPB:
	ld	a,12
	scf
	ret


;-----------------------------------------------------------------------------
;
; Return format choice string
;
;Input:   -
;Output:  HL = Address of the choice string in the kernel slot

DRV_CHOICE:
	ld      hl,NULL_MSG
	ret


;-----------------------------------------------------------------------------
;
; Format a disk
;
;Input:   A  = Formatting choice, from 1 to 9 (see DRV_CHOICE).
;         D  = Drive number, starting at 0
;         HL = Address of work area in memory
;         DE = Size of work area
;Output:  Cy = 0 on success, 1 on error
;         A  = Error code (on error only):
;              0   Write protected
;              2   Not ready
;              4   Data (CRC) error
;              6   Seek error
;              8   Record not found
;              10  Write fault
;              12  Bad parameter
;              14  Insufficient memory
;              16  Other errors

DRV_DSKFMT:
	ld	a,16
	scf
	ret


;-----------------------------------------------------------------------------
;
; Turn off the floppy disk drive motor
;
;Input:   -
;Output:  -

DRV_MTOFF:
	ret

endif


;=====
;=====  END of DRIVE-BASED specific routines
;=====


;=====
;=====  BEGIN of DEVICE-BASED specific routines
;=====

if DRV_TYPE eq 1

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
          B = Number of sectors actually read (in case of error only)

DEV_RW:
	ld	a,.NRDY
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
;+1 (1): Removable device/logical unit flags:
;           bit 0: The device supports hot-plug.
;                  Ignored if hot-plug bit in DRV_FLAGS is zero.
;           bit 1: Logical unit 1 is removable.
;           ...
;           bit 7: Logical unit 7 is removable.
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
	ld	(hl),0
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

DEV_STATUS:
	xor	a
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
	ld	a,.IFORM
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

endif


;=====
;=====  END of DEVICE-BASED specific routines
;=====


;-----------------------------------------------------------------------------
;
; Padding up to the required driver size

DRV_END:

	;ds	3FD0h-(DRV_END-DRV_START)

	end