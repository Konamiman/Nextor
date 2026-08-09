; Nextor disk driver template.
; See the TODO comments for implementation/customization points.

	;TODO: If this is a driver intended to be loaded in RAM, change to 1.
	;If this is a ROM driver, see the Makefile for building; otherwise,
	;assembling the file directly with N80 is enough:
	;  N80 driver.asm mydriver.drv --include-directory <path to the Nextor SDK>
RAM_DRIVER: equ 0

	;Note that these file paths are relative to the root of the Nextor SDK.
	INCLUDE asm/macros/undoc.inc	;Use these instead of undocumented Z80 instructions
	INCLUDE asm/constants/driver_result_codes.inc
	INCLUDE asm/constants/dos_errors.inc
	INCLUDE asm/constants/rom_bank_header.inc

	module DRIVER_QUERY
	INCLUDE asm/constants/driver_driver_queries.inc
	endmod

	module DEVICE_QUERY
	INCLUDE asm/constants/driver_device_queries.inc
	endmod


	;*********************
	;***  DRIVER CODE  ***
	;*********************

	; mknexrom expects the driver file to start with 256 dummy bytes
	; (it overwrites them with the kernel's common bank header code), so
	; the actual driver code starts at 4100h. RAM drivers are loaded at
	; 4100h directly, without the dummy area.

	if RAM_DRIVER eq 0

	org 4000h
	ds 4100h-$,0

	else

	org 4100h

	endif
	

DRIVER_START:

	;Driver signature

	db	"NEXTORv3_DRIVER",0

	;Jump table

	jp	TIMER_INT
	jp	OEMSTAT
	jp	BASDEV
	jp	EXTBIO
	jp	DRIVER_QUERY
	jp	DEVICE_QUERY
	jp	CUSTOM_DRIVER_QUERY
	jp  CUSTOM_DEVICE_QUERY
	jp	READ_WRITE
	jp	RESERVED_0
	jp	RESERVED_1
	jp	RESERVED_2

	if RAM_DRIVER eq 0

	jp	DIRECT_0
	jp	DIRECT_1
	jp	DIRECT_2
	jp	DIRECT_3
	jp	DIRECT_4

	endif


	;--- Timer interrupt routine
	;
	;    TODO: implement if your driver needs to run code on every VBLANK
	;    interrupt; it is only called if you set the TIMER_INT flag in
	;    driver query 3 ("get driver initialization parameters").

TIMER_INT:
	ret	;TIMER_INT
	ret
	ret

	;--- Handler for BASIC expanded statement ("CALL") handler.
	;    Works the expected way, except that CALBAS in kernel page 0
	;    must be called instead of CALBAS in MSX BIOS.
	;
	;    TODO: implement if your driver provides its own CALL commands;
	;    otherwise leave as-is (Cy set = statement not recognized).

OEMSTAT:
	scf
	ret
	ret


	;--- Handler for BASIC expanded devices.
	;    Works the expected way, but see CALBAS exception for STATEMENT.
	;
	;    TODO: implement if your driver provides BASIC expanded devices;
	;    otherwise leave as-is.

BASDEV:
	scf
	ret
	ret


	;--- Extended BIOS hook.
	;    Works the expected way, except that it must return
	;    IYl=1 if the old hook must be called, IYl=0 otherwise.
	;    Only called if the driver has returned EXTBIO flag set
	;    in the "get driver initialization parameters" query.
	;
	;    TODO: implement if your driver hooks the extended BIOS;
	;    otherwise leave as-is.
EXTBIO:
	ret
	ret
	ret


	;* Jump table entries reserved for future use.

RESERVED_0:
RESERVED_1:
RESERVED_2:
	ret


	if RAM_DRIVER eq 0

	;* Direct calls entry points.
	;  There is a jump table at address 7850h in ROM banks 0 and 3,
	;  that will be redirected here.
	;
	;  TODO: implement any driver-specific routines you want to expose
	;  to external programs (your own configuration/flashing tools, etc).

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

	endif


	;--- Driver query
	;    Input:  A = Query index
	;            F, BC, DE, HL = Depends on the query
	;    Output: A = Error code:
	;                RESULT_OK: success
	;                RESULT_NOT_IMPLEMENTED: query not implemented
	;                Others: depends on the query
	;            F, BC, DE, HL = Depends on the query

DRIVER_QUERY:
	dec a
	jp z,DO_DRVQ_GET_VERSION
	dec a
	jp z,DO_DRVQ_GET_STRING

	if RAM_DRIVER eq 0
		dec a
		jp z,DO_DRVQ_GET_INIT_PARAMS
		dec a
		jp z,DO_DRVQ_INIT
	else
		dec a
		dec a
	endif

	dec a
	jp z,DO_DRVQ_GET_MAX_DEVICE
	
	if RAM_DRIVER eq 1
		dec a
		jp z,DO_DRVQ_INIT_RAM
		dec a
		jp z,DO_DRVQ_SHUTDOWN_RAM
	endif

	ld a,RESULT_NOT_IMPLEMENTED
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
	ld bc,0100h	;TODO: your driver's version, B.C.D (this is 1.0.0)
	ld d,0
	xor a
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
	;TODO: provide the other strings (hardware name, hardware author,
	;serial number) if they make sense for your driver.
	ld a,b
	ld b,d
	ex de,hl
	dec a
	ld hl,STR_DRIVER_NAME
	jp z,OUTPUT_STRING
	dec a
	ld hl,STR_DRIVER_AUTHOR
	jp z,OUTPUT_STRING
	ld a,RESULT_NOT_IMPLEMENTED
	ret

;TODO: replace with your driver name and your name.
STR_DRIVER_NAME: db "My driver",0
STR_DRIVER_AUTHOR: db "Myself",0


	if RAM_DRIVER eq 0

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
; Note: this is the same as Nextor 2 DRV_INIT when called with A=0, except that
; TIMER_INT flag is returned in B, not in Cy; an error code is returned in A;
; and DE is passed at input.

DO_DRVQ_GET_INIT_PARAMS:
	;TODO: set B to request the TIMER_INT/EXTBIO hooks and HL to request
	;page 3 work area if your driver needs them.
	xor a
	ld b,0
	ld hl,0
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
; Note: this is the same as Nextor 2 DRV_INIT when called with A=1, except that number of
; allocated drives is not passed in B, DE is passed at input, and an error code can be returned.

DO_DRVQ_INIT:
	;TODO: detect and initialize your hardware here; return RESULT_INIT_ERROR
	;if it's absent or fails to initialize. As-is it just prints INIT_MSG
	;using the print-character routine passed in DE.
	ld hl,INIT_MSG
	call PRINT_WITH_DE
	xor a	;RESULT_OK
	ret

	endif


; Driver query 5: Get maximum supported device number
;
; Input:  -
; Output: A = RESULT_OK or RESULT_NOT_IMPLEMENTED
;         B = Maximum supported device number
;
; RESULT_NOT_IMPLEMENTED is equivalent to returning RESULT_OK and B=4.

DO_DRVQ_GET_MAX_DEVICE:
	;TODO: return in B the highest device number your driver handles
	;(devices are numbered starting at 1); as-is, 4 is assumed.
	ld a,RESULT_NOT_IMPLEMENTED
	ret


	if RAM_DRIVER eq 1

;Driver query 6: Initialize RAM driver
;
;Input:  DE = Address of a routine for printing a character
;        B  = RAM slot number where the driver is located
;        C  = RAM segment number where the driver is located
;Output: A  = RESULT_OK, RESULT_INIT_ERROR or RESULT_NOT_IMPLEMENTED
;        B  = Flags
;             0: TIMER_INT should be hooked
;             1: EXTBIO should be hooked
;             2-7: Must be zero
;
; If RESULT_NOT_IMPLEMENTED is returned, B=0 is assumed.

DO_DRVQ_INIT_RAM:
	;TODO: initialize your driver here (B and C tell you the slot and
	;segment it was loaded in); return RESULT_INIT_ERROR on failure.
	;As-is it just prints INIT_MSG using the print routine passed in DE.
	ld hl,INIT_MSG
	call PRINT_WITH_DE
	xor a	;RESULT_OK
	ld b,0	;no TIMER_INT/EXTBIO hooks
	ret


; Driver query 7: Shutdown RAM driver
;
; Input:  DE = Address of a routine for printing a character
; Output: A  = RESULT_OK or RESULT_NOT_IMPLEMENTED

DO_DRVQ_SHUTDOWN_RAM:
	;TODO: clean up here before the driver gets unloaded.
	;As-is it just prints SHUTDOWN_MSG using the print routine passed in DE.
	ld hl,SHUTDOWN_MSG
	call PRINT_WITH_DE
	xor a	;RESULT_OK
	ret

	endif


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
	dec a
	jr z,DO_DEVQ_GET_STRING
	dec a
	jr z,DO_DEVQ_GET_PARAMS
	dec a
	jr z,DO_DEVQ_GET_STATUS
	dec a
	jr z,DO_DEVQ_GET_AVAILABILITY
	dec a
	jr z,DO_DEVQ_GET_FORMAT_CHOICES
	dec a
	jr z,DO_DEVQ_DO_FORMAT
	dec a
	jr z,DO_DEVQ_STOP_MOTOR
	ld a,RESULT_NOT_IMPLEMENTED
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
	;TODO: return the requested string for each of your devices
	;(see DO_DRVQ_GET_STRING above for the OUTPUT_STRING pattern).
	ld a,RESULT_INVALID_DEVICE
	ret


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
;         bit 1: 1 if the device is read only. A device that can dynamically
;                  be write protected or write enabled is not considered
;                  to be read-only.
;         bit 2: 1 if the device is a floppy disk drive.
;         bit 3: 1 if this device shouldn't be used for automapping.
;         bits 4-7: must be zero.
; +8 (2): Number of cylinders
; +10 (1): Number of heads
; +11 (1): Number of sectors per track
;
;RESULT_NOT_IMPLEMENTED is interpreted as a block device with 512 byte sectors, unknown total number of sectors, and flags equal to 0.
;
;This is the same as Nextor 2 LUN_INFO, but device id is passed in C instead of A, there's no LUN index, and error codes differ.
;Also HL=0 at input must be supported.

DO_DEVQ_GET_PARAMS:
	;TODO: fill the buffer at HL with each device's parameters. As-is
	;(RESULT_INVALID_DEVICE for every device, like all the device queries
	;in this dummy driver) the driver exposes no usable devices.
	ld a,RESULT_INVALID_DEVICE
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
	;TODO: report status for each of your devices (for fixed devices,
	;returning RESULT_NOT_IMPLEMENTED for existing devices is enough).
	ld a,RESULT_INVALID_DEVICE
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
; RESULT_NOT_IMPLEMENTED is interpreted as returning B=1.
;
; Note: this is the same as "Get device status" but it only returns B=0 or B=1,
; and it does not change the internal "changed" status of the device.

DO_DEVQ_GET_AVAILABILITY:
	;TODO: report availability for each of your devices (only meaningful
	;for removable devices, same as "get device status").
	ld a,RESULT_INVALID_DEVICE
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
	;TODO: implement only if your devices are formattable floppy disk
	;drives; otherwise leave as-is (also queries 6 and 7 below).
	ld a,RESULT_NOT_IMPLEMENTED
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
	ld a,RESULT_NOT_IMPLEMENTED
	ret


; Device query 7: Stop the floppy disk drive motor
;
; Input:  -
; Output: RESULT_OK: ok, motor has been stopped
;         RESULT_INVALID_DEVICE: device does not exist
;         RESULT_NOT_IMPLEMENTED: the device is not a floppy disk
;                                or stopping the drive motor is not supported

DO_DEVQ_STOP_MOTOR:
	ld a,RESULT_NOT_IMPLEMENTED
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
	ld a,RESULT_NOT_IMPLEMENTED
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
	ld a,RESULT_NOT_IMPLEMENTED
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
    ;                  .IDEVN: Invalid device number
    ;                  .NRDY: Not ready
    ;                  .DISK: General unknown disk error
    ;                  .DATA: CRC error when reading
    ;                  .RNF: Sector not found
    ;                  .UFORM: Unformatted disk
    ;                  .WPROT: Write protected media, or read-only device
    ;                  .WRERR: Write error
    ;                  .NCOMP: Incompatible disk.
    ;                  .SEEK: Seek error.
	;               B = Sectors successfully transferred

READ_WRITE:
	;TODO: implement the actual sector transfer with your hardware.
	;This is the heart of the driver: everything else describes devices,
	;this routine moves the data.
	ld a,.IDEVN
	ret


	INCLUDE asm/code/output_string.asm


	;--- Print a zero-terminated string via a character output routine
	;    Input: HL = string, DE = character output routine address
	;    Trashes: AF, HL, IX

PRINT_WITH_DE:
	push de
	ld de,0C300h	;JP opcode + 00
	push de
	ld ix,1
	add ix,sp	;IX -> JP <charout> trampoline on stack
	call PRINT_HL
	pop de
	pop de
	ret

PRINT_HL:
	ld a,(hl)
	or a
	ret z
	call JP_IX
	inc hl
	jr PRINT_HL

JP_IX: jp (ix)

	.stresc on

;TODO: replace with the message your driver prints when initialized.
INIT_MSG: db "\r\nMy driver\r\n"
          db "by Myself\r\n",0

	if RAM_DRIVER eq 1

;TODO: replace with the message your driver prints when unloaded.
SHUTDOWN_MSG: db "My driver unloaded\r\n",0

	endif

	if RAM_DRIVER eq 0

	;Pad the ROM driver up to the bank switching code area at 7FD0h
	;(not needed for RAM drivers, which are plain loadable files).
	;This also makes the assembly fail if the driver outgrows the bank.
	ds 7FD0h-$,0FFh

	endif

	end
