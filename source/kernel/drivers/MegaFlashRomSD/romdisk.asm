;-----------------------------------------------------------------------------
; ROM Disk driver
;
; 2013/30/01	Manuel Pazos
;-----------------------------------------------------------------------------

BANK4		equ	#6000		; address to select megarom bank in #4000-#5FFF
BANK6		equ	#6800		; address to select megarom bank in #6000-#7FFF

DSK_BANK	equ	#10+#80		; Bank number of DSK
DRIVER_BANK	equ	#07*2		; Device driver bank number

UPB_SSZ		equ 0Bh
UPB_SFAT	equ 16h
UPB_NDIR	equ 11h

;-----------------------------------------------------------------------------
; Check if there is a DSK installed as ROM disk
; Out: Z = Available, NZ = Not available
;-----------------------------------------------------------------------------
RomDiskCheck:
		di
		ld	a,DSK_BANK
		ld	(BANK4),a
		
		ld	hl,#4000
		ld	de,UPB_SSZ
		add	hl,de
		ld	d,(hl)			;Sector size must always
		inc	hl			; be 200h.
		ld	a,(hl)
		inc	hl
		sub	2
		or	d
		jp	nz,upb_not_found
		;
		or	(hl)
		jp	z,upb_not_found		;Sectors/cluster must be
		neg				; a non-zero power of 2.
		and	(hl)
		cp	(hl)
		inc	hl
		jp	nz,upb_not_found
		;
		inc	hl			;Ignore number of reserved
		inc	hl			; sectors
		;
		ld	a,(hl)
		dec	a				;Number of FATs must be
		cp	7				; 1...7 (nobody has more
		jp	nc,upb_not_found		; than 7 copies of the FAT!)
		;
		inc	hl				;Zero directory entries
		ld	a,(hl)			;means FAT32 drive.
		inc	hl
		or	(hl)
		jr	z,upb_not_found
		
		ld	de,UPB_SFAT-(UPB_NDIR+1)	;Ignore "total sectors"
		add	hl,de			;  and "media byte".
		
		ld	e,(hl)			;Number of sectors per
		inc	hl				; FAT less than 256.
		ld	d,(hl)
		dec	de
		inc	d
		dec	d
		jr	nz,upb_not_found
		
upb_found:
		ld	a,DRIVER_BANK
		ld	(BANK4),a
		xor	a
		ret				; if UPB was OK.
		
upb_not_found:					;If anything was wrong with
		ld	a,DRIVER_BANK
		ld	(BANK4),a
		or	h			; the UPB then return with
		ret				; Z-flag clear.

;-----------------------------------------------------------------------------
; Read sectors
;	B    = Number of sectors to read
;	(DE) = First sector number to read
;	HL   = destination address for the transfer
;-----------------------------------------------------------------------------
RomDiskRead:
		di
		ld	a,(de)
		ld	c,a
		inc	de
		ld	a,(de)
		ld	d,a
		ld	e,c	; DE = Sector number.
				; Last two bytes of sector number are ignored.
				; (720KB disk = 1440 sectors)
.loop:
		push	bc
		push	de
		push	hl
		
		call	.readSector
		
		pop	hl
		inc	h
		inc	h
		pop	de
		inc	de
		pop	bc
		djnz	.loop

		ld	a,DRIVER_BANK
		ld	(BANK4),a
		ei

		xor	a			; Ok
		ret

.readSector:
		push	hl
		ld	c,e
		
		srl	d
		rr	e
		srl	d
		rr	e
		srl	d
		rr	e
		srl	d
		rr	e			; Sector / 16 = banco de 8k que lo contiene
		ld	a,e
		add	a,DSK_BANK		; Omite las paginas usadas por Nextor
		ld	(BANK4),a		; Cambia la p�gina de la ROM
		
		ld	a,c
		and	#0F
		add	a,a
		add	a,#40
		ld	h,a
		ld	l,#00
		
		pop	de			; Direcci�n de destino
		jp	transfer

;-----------------------------------------------------------------------------
;
; Device information gathering
;
;Input:   A = Device index 0
;         B = Information to return:
;             0: Basic information
;             1: Manufacturer name string
;             2: Device name string
;             3: Serial number string
;         HL = Pointer to a buffer in RAM
;         D  = Buffer size (added for Nextor 3)
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
;-----------------------------------------------------------------------------
RomDiskInfo:
		ld a,d

		djnz	.INFO2
		; 1: Manufacturer name string	
		ld	de,TXT_MANU
		jr	.end
	
	
.INFO2:
		djnz	.INFO3
		; 2: Device name string
		ld	de,TXT_DEV
		jr	.end
	
.INFO3:
		djnz	.INFO_ERR
		; 3: Serial number string
		ld	de,TXT_SERIAL
		jr	.end

.INFO_ERR:
		ld a,2
		ret

.end:
		ex	de,hl
		ld	b,a
		jp OUTPUT_STRING


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
RomDiskStatus:
		call	GETWRK
		bit	BIT_ROM_DSK,(IX+0)	; Is ROM disk available?
		jr	z,.error

		dec	b
		jr	nz,.error
		
		ld	a,1			; Device available and has not changed
		ret
.error:
		xor	a			; Device not available
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
RomDiskLUN_INFO:
		;dec	b
		;jp	nz,LUN_ERROR
		
		call	GETWRK
		bit	BIT_ROM_DSK,(IX+0)	; Is ROM disk available?
		jr	z,.error
		
		ld	(hl),0		; +0: Medium type = Block device
		inc	hl
		
		ld	(hl),0		; +1: Sector size = #200
		inc	hl
		ld	(hl),2
		inc	hl
		
		ld	bc,1440		; Number of sector on a 720K FDD
		ld	(hl),c		; +3: Total number of sectors
		inc	hl
		ld	(hl),b
		inc	hl
		ld	(hl),0
		inc	hl
		ld	(hl),0
		inc	hl
        	
		ld	(hl),%011	; +7: Flags ;!!!!!!!
		inc	hl
		
		ld	(hl),0		; +8 Cylinders
		inc	hl
		ld	(hl),0
		inc	hl
		
		ld	(hl),0		; +10: Heads
		inc	hl
		
		ld	(hl),0		; +11: Sectors per tracks
		
		xor a			; Ok, buffer filled with information.
		ret
	
.error:
		ld	a,1		; Error
		ret
;-----------------------------------------------------------------------------
; Texts
;-----------------------------------------------------------------------------
TXT_MANU:	db	"Manuel Pazos    ",0
TXT_DEV:	db	"MFRSD ROM Disk  ",0
TXT_SERIAL:	db	"00-00-00-01     ",0