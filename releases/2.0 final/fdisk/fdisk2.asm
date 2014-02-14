;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.3.0 #8604 (May 11 2013) (MINGW32)
; This file was generated Tue Feb 11 14:28:56 2014
;--------------------------------------------------------
	.module fdisk2
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _sectorsPerTrack
	.globl _mainExtendedPartitionFirstSector
	.globl _mainExtendedPartitionSectorCount
	.globl _nextDeviceSector
	.globl _partitions
	.globl _partitionsCount
	.globl _selectedLunIndex
	.globl _deviceIndex
	.globl _driverSlot
	.globl _regs
	.globl _OUT_FLAGS
	.globl _ASMRUT
	.globl _sectorBufferBackup
	.globl _sectorBuffer
	.globl _remote_CreateFatFileSystem
	.globl _CreateFatFileSystem
	.globl _CreateFatBootSector
	.globl _GetNewSerialNumber
	.globl _ClearSectorBuffer
	.globl _SectorBootCode
	.globl _remote_CalculateFatFileSystemParameters
	.globl _CalculateFatFileSystemParameters
	.globl _CalculateFatFileSystemParametersFat12
	.globl _CalculateFatFileSystemParametersFat16
	.globl _WriteSectorToDevice
	.globl _remote_PreparePartitionningProcess
	.globl _remote_CreatePartition
	.globl _CreatePartition
	.globl _putchar
	.globl _Locate
	.globl _DriverCall
	.globl _DosCall
	.globl _SwitchSystemBankThenCall
	.globl _AsmCallAlt
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_sectorBuffer::
	.ds 512
_sectorBufferBackup::
	.ds 512
_ASMRUT::
	.ds 4
_OUT_FLAGS::
	.ds 1
_regs::
	.ds 12
_driverSlot::
	.ds 1
_deviceIndex::
	.ds 1
_selectedLunIndex::
	.ds 1
_partitionsCount::
	.ds 2
_partitions::
	.ds 2
_nextDeviceSector::
	.ds 4
_mainExtendedPartitionSectorCount::
	.ds 4
_mainExtendedPartitionFirstSector::
	.ds 4
_sectorsPerTrack::
	.ds 2
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;fdisk2.c:61: int main(int bc, int hl)
;	---------------------------------
; Function main
; ---------------------------------
_main_start::
_main:
;fdisk2.c:63: ASMRUT[0] = 0xC3;   //Code for JP
	ld	hl,#_ASMRUT
	ld	(hl),#0xC3
;fdisk2.c:64: switch(bc) {
	ld	iy,#2
	add	iy,sp
	ld	a,0 (iy)
	sub	a, #0x01
	ld	a,1 (iy)
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	C,00105$
	ld	a,#0x04
	cp	a, 0 (iy)
	ld	a,#0x00
	sbc	a, 1 (iy)
	jp	PO, 00115$
	xor	a, #0x80
00115$:
	jp	M,00105$
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	add	a,#0xFF
	ld	e,a
;fdisk2.c:66: return remote_CalculateFatFileSystemParameters((byte*)hl);
	ld	iy,#4
	add	iy,sp
	ld	c,0 (iy)
	ld	b,1 (iy)
;fdisk2.c:64: switch(bc) {
	ld	d,#0x00
	ld	hl,#00116$
	add	hl,de
	add	hl,de
;fdisk2.c:65: case f_CalculateFatFileSystemParameters:
	jp	(hl)
00116$:
	jr	00101$
	jr	00102$
	jr	00103$
	jr	00104$
00101$:
;fdisk2.c:66: return remote_CalculateFatFileSystemParameters((byte*)hl);
	push	bc
	call	_remote_CalculateFatFileSystemParameters
	pop	af
	ret
;fdisk2.c:68: case f_CreateFatFileSystem:
00102$:
;fdisk2.c:69: return remote_CreateFatFileSystem((byte*)hl);
	push	bc
	call	_remote_CreateFatFileSystem
	pop	af
	ret
;fdisk2.c:71: case f_PreparePartitionningProcess:
00103$:
;fdisk2.c:72: return remote_PreparePartitionningProcess((byte*)hl);
	push	bc
	call	_remote_PreparePartitionningProcess
	pop	af
	ret
;fdisk2.c:74: case f_CreatePartition:
00104$:
;fdisk2.c:75: return remote_CreatePartition((byte*)hl);
	push	bc
	call	_remote_CreatePartition
	pop	af
	ret
;fdisk2.c:77: default:
00105$:
;fdisk2.c:78: return 0;
	ld	hl,#0x0000
;fdisk2.c:79: }
	ret
_main_end::
;fdisk2.c:83: int remote_CreateFatFileSystem(byte* callerParameters)
;	---------------------------------
; Function remote_CreateFatFileSystem
; ---------------------------------
_remote_CreateFatFileSystem_start::
_remote_CreateFatFileSystem:
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-8
	add	hl,sp
	ld	sp,hl
;fdisk2.c:90: *((ulong*)&callerParameters[7]));
	ld	a,4 (ix)
	add	a, #0x07
	ld	e,a
	ld	a,5 (ix)
	adc	a, #0x00
	ld	d,a
	ld	hl, #0x0004
	add	hl, sp
	ex	de, hl
	ld	bc, #0x0004
	ldir
;fdisk2.c:89: *((ulong*)&callerParameters[3]),
	ld	e,4 (ix)
	ld	d,5 (ix)
	inc	de
	inc	de
	inc	de
	ld	hl, #0x0000
	add	hl, sp
	ex	de, hl
	ld	bc, #0x0004
	ldir
;fdisk2.c:88: callerParameters[2],
	ld	c,4 (ix)
	ld	b,5 (ix)
	push	bc
	pop	iy
	ld	b,2 (iy)
;fdisk2.c:87: callerParameters[1],
	ld	e,1 (iy)
;fdisk2.c:86: callerParameters[0],
	ld	d, 0 (iy)
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	push	hl
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	ld	l,-6 (ix)
	ld	h,-5 (ix)
	push	hl
	ld	l,-8 (ix)
	ld	h,-7 (ix)
	push	hl
	push	bc
	inc	sp
	ld	a,e
	push	af
	inc	sp
	push	de
	inc	sp
	call	_CreateFatFileSystem
	ld	iy,#0x000B
	add	iy,sp
	ld	sp,iy
	ld	h,#0x00
	ld	sp,ix
	pop	ix
	ret
_remote_CreateFatFileSystem_end::
;fdisk2.c:94: byte CreateFatFileSystem(byte driverSlot, byte deviceIndex, byte lunIndex, ulong firstDeviceSector, ulong fileSystemSizeInK)
;	---------------------------------
; Function CreateFatFileSystem
; ---------------------------------
_CreateFatFileSystem_start::
_CreateFatFileSystem:
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-37
	add	hl,sp
	ld	sp,hl
;fdisk2.c:102: CalculateFatFileSystemParameters(fileSystemSizeInK, &parameters);
	ld	hl,#0x0004
	add	hl,sp
	ld	-2 (ix),l
	ld	-1 (ix),h
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	push	hl
	ld	l,13 (ix)
	ld	h,14 (ix)
	push	hl
	ld	l,11 (ix)
	ld	h,12 (ix)
	push	hl
	call	_CalculateFatFileSystemParameters
	ld	hl,#0x0006
	add	hl,sp
	ld	sp,hl
;fdisk2.c:106: CreateFatBootSector(&parameters);
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	push	hl
	call	_CreateFatBootSector
	pop	af
;fdisk2.c:108: if((error = WriteSectorToDevice(driverSlot, deviceIndex, lunIndex, firstDeviceSector)) != 0) {
	ld	l,9 (ix)
	ld	h,10 (ix)
	push	hl
	ld	l,7 (ix)
	ld	h,8 (ix)
	push	hl
	ld	h,6 (ix)
	ld	l,5 (ix)
	push	hl
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_WriteSectorToDevice
	pop	af
	pop	af
	pop	af
	inc	sp
	ld	-3 (ix), l
	ld	-4 (ix), l
	ld	a,-3 (ix)
	or	a, a
	jr	Z,00102$
;fdisk2.c:109: return error;
	ld	l,-4 (ix)
	jp	00115$
00102$:
;fdisk2.c:114: ClearSectorBuffer();
	call	_ClearSectorBuffer
;fdisk2.c:115: zeroSectorsToWrite = (parameters.sectorsPerFat * FAT_COPIES) + (parameters.sectorsPerRootDirectory) - 1;
	ld	a,-2 (ix)
	add	a, #0x0A
	ld	-6 (ix),a
	ld	a,-1 (ix)
	adc	a, #0x00
	ld	-5 (ix),a
	ld	l,-6 (ix)
	ld	h,-5 (ix)
	ld	a,(hl)
	ld	-8 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-7 (ix),a
	sla	-8 (ix)
	rl	-7 (ix)
	ld	a,-2 (ix)
	ld	-10 (ix),a
	ld	a,-1 (ix)
	ld	-9 (ix),a
	ld	l,-10 (ix)
	ld	h,-9 (ix)
	ld	de, #0x000D
	add	hl, de
	ld	a,(hl)
	ld	-10 (ix), a
	ld	-10 (ix),a
	ld	-9 (ix),#0x00
	ld	a,-10 (ix)
	add	a, -8 (ix)
	ld	-10 (ix),a
	ld	a,-9 (ix)
	adc	a, -7 (ix)
	ld	-9 (ix),a
	ld	a,-10 (ix)
	add	a,#0xFF
	ld	-35 (ix),a
	ld	a,-9 (ix)
	adc	a,#0xFF
	ld	-34 (ix),a
;fdisk2.c:116: sectorNumber = firstDeviceSector + 2;
	ld	a,7 (ix)
	add	a, #0x02
	ld	-14 (ix),a
	ld	a,8 (ix)
	adc	a, #0x00
	ld	-13 (ix),a
	ld	a,9 (ix)
	adc	a, #0x00
	ld	-12 (ix),a
	ld	a,10 (ix)
	adc	a, #0x00
	ld	-11 (ix),a
;fdisk2.c:117: for(i = 0; i < zeroSectorsToWrite; i++) {
	ld	hl,#0x0000
	ex	(sp), hl
00113$:
	ld	a,-37 (ix)
	sub	a, -35 (ix)
	ld	a,-36 (ix)
	sbc	a, -34 (ix)
	jr	NC,00105$
;fdisk2.c:118: if((error = WriteSectorToDevice(driverSlot, deviceIndex, lunIndex, sectorNumber)) != 0) {
	ld	l,-12 (ix)
	ld	h,-11 (ix)
	push	hl
	ld	l,-14 (ix)
	ld	h,-13 (ix)
	push	hl
	ld	h,6 (ix)
	ld	l,5 (ix)
	push	hl
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_WriteSectorToDevice
	pop	af
	pop	af
	pop	af
	inc	sp
	ld	a,l
	ld	-4 (ix),a
	or	a, a
	jr	Z,00104$
;fdisk2.c:119: return error;
	ld	l,-4 (ix)
	jp	00115$
00104$:
;fdisk2.c:121: sectorNumber++;
	inc	-14 (ix)
	jr	NZ,00140$
	inc	-13 (ix)
	jr	NZ,00140$
	inc	-12 (ix)
	jr	NZ,00140$
	inc	-11 (ix)
00140$:
;fdisk2.c:117: for(i = 0; i < zeroSectorsToWrite; i++) {
	inc	-37 (ix)
	jr	NZ,00113$
	inc	-36 (ix)
	jr	00113$
00105$:
;fdisk2.c:126: sectorBuffer[0] = 0xF0;
	ld	hl,#_sectorBuffer
	ld	(hl),#0xF0
;fdisk2.c:127: sectorBuffer[1] = 0xFF;
	ld	hl,#(_sectorBuffer + 0x0001)
	ld	(hl),#0xFF
;fdisk2.c:128: sectorBuffer[2] = 0xFF;
	ld	hl,#(_sectorBuffer + 0x0002)
	ld	(hl),#0xFF
;fdisk2.c:129: if(parameters.isFat16) {
	ld	a,-2 (ix)
	ld	-14 (ix),a
	ld	a,-1 (ix)
	ld	-13 (ix),a
	ld	l,-14 (ix)
	ld	h,-13 (ix)
	ld	de, #0x000E
	add	hl, de
	ld	a,(hl)
	or	a, a
	jr	Z,00107$
;fdisk2.c:130: sectorBuffer[3] = 0xFF;
	ld	hl,#_sectorBuffer + 3
	ld	(hl),#0xFF
00107$:
;fdisk2.c:132: if((error = WriteSectorToDevice(driverSlot, deviceIndex, lunIndex, firstDeviceSector + 1)) != 0) {
	ld	a,7 (ix)
	add	a, #0x01
	ld	-14 (ix),a
	ld	a,8 (ix)
	adc	a, #0x00
	ld	-13 (ix),a
	ld	a,9 (ix)
	adc	a, #0x00
	ld	-12 (ix),a
	ld	a,10 (ix)
	adc	a, #0x00
	ld	-11 (ix),a
	ld	l,-12 (ix)
	ld	h,-11 (ix)
	push	hl
	ld	l,-14 (ix)
	ld	h,-13 (ix)
	push	hl
	ld	h,6 (ix)
	ld	l,5 (ix)
	push	hl
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_WriteSectorToDevice
	pop	af
	pop	af
	pop	af
	inc	sp
	ld	a,l
	ld	-4 (ix),a
	or	a, a
	jr	Z,00109$
;fdisk2.c:133: return error;
	ld	l,-4 (ix)
	jr	00115$
00109$:
;fdisk2.c:135: if((error = WriteSectorToDevice(driverSlot, deviceIndex, lunIndex, firstDeviceSector + 1 + parameters.sectorsPerFat)) != 0) {
	ld	l,-6 (ix)
	ld	h,-5 (ix)
	ld	a,(hl)
	ld	-10 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-9 (ix),a
	ld	a,-10 (ix)
	ld	-18 (ix),a
	ld	a,-9 (ix)
	ld	-17 (ix),a
	ld	-16 (ix),#0x00
	ld	-15 (ix),#0x00
	ld	a,-14 (ix)
	add	a, -18 (ix)
	ld	-18 (ix),a
	ld	a,-13 (ix)
	adc	a, -17 (ix)
	ld	-17 (ix),a
	ld	a,-12 (ix)
	adc	a, -16 (ix)
	ld	-16 (ix),a
	ld	a,-11 (ix)
	adc	a, -15 (ix)
	ld	-15 (ix),a
	ld	l,-16 (ix)
	ld	h,-15 (ix)
	push	hl
	ld	l,-18 (ix)
	ld	h,-17 (ix)
	push	hl
	ld	h,6 (ix)
	ld	l,5 (ix)
	push	hl
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_WriteSectorToDevice
	pop	af
	pop	af
	pop	af
	inc	sp
	ld	a,l
	ld	-4 (ix),a
	or	a, a
	jr	Z,00111$
;fdisk2.c:136: return error;
	ld	l,-4 (ix)
	jr	00115$
00111$:
;fdisk2.c:141: return 0;
	ld	l,#0x00
00115$:
	ld	sp,ix
	pop	ix
	ret
_CreateFatFileSystem_end::
;fdisk2.c:145: void CreateFatBootSector(dosFilesystemParameters* parameters)
;	---------------------------------
; Function CreateFatBootSector
; ---------------------------------
_CreateFatBootSector_start::
_CreateFatBootSector:
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
	dec	sp
;fdisk2.c:147: fatBootSector* sector = (fatBootSector*)sectorBuffer;
;fdisk2.c:149: ClearSectorBuffer();
	call	_ClearSectorBuffer
;fdisk2.c:151: sector->jumpInstruction[0] = 0xEB;
	ld	hl,#_sectorBuffer
	ld	(hl),#0xEB
;fdisk2.c:152: sector->jumpInstruction[1] = 0xFE;
	inc	hl
	ld	(hl),#0xFE
;fdisk2.c:153: sector->jumpInstruction[2] = 0x90;
	ld	hl,#_sectorBuffer + 2
	ld	(hl),#0x90
;fdisk2.c:154: strcpy(sector->oemNameString, "NEXTOR20");
	ld	hl,#__str_0
	ld	de,#(_sectorBuffer + 0x0003)
	xor	a, a
00114$:
	cp	a, (hl)
	ldi
	jr	NZ, 00114$
;fdisk2.c:155: sector->sectorSize = 512;
	ld	hl,#0x0200
	ld	((_sectorBuffer + 0x000b)), hl
;fdisk2.c:156: sector->sectorsPerCluster = parameters->sectorsPerCluster;
	ld	a,4 (ix)
	ld	-2 (ix),a
	ld	a,5 (ix)
	ld	-1 (ix),a
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	ld	de, #0x000C
	add	hl, de
	ld	a,(hl)
	ld	(#(_sectorBuffer + 0x000d)),a
;fdisk2.c:157: sector->reservedSectors = 1;
	ld	hl,#0x0001
	ld	((_sectorBuffer + 0x000e)), hl
;fdisk2.c:158: sector->numberOfFats = FAT_COPIES;
	ld	hl,#_sectorBuffer + 16
	ld	(hl),#0x02
;fdisk2.c:159: sector->rootDirectoryEntries = parameters->sectorsPerRootDirectory * DIR_ENTRIES_PER_SECTOR;
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	ld	de, #0x000D
	add	hl, de
	ld	l,(hl)
	ld	h,#0x00
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	ex	de,hl
	ld	((_sectorBuffer + 0x0011)), de
;fdisk2.c:160: if((parameters->totalSectors & 0xFFFF0000) == 0) {
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	c,(hl)
	inc	hl
	ld	h,(hl)
	ld	a,c
	or	a,a
	jr	NZ,00102$
	or	a,h
	jr	NZ,00102$
;fdisk2.c:161: sector->smallSectorCount = parameters->totalSectors;
	ld	((_sectorBuffer + 0x0013)), de
00102$:
;fdisk2.c:163: sector->mediaId = 0xF0;
	ld	hl,#_sectorBuffer + 21
	ld	(hl),#0xF0
;fdisk2.c:164: sector->sectorsPerFat = parameters->sectorsPerFat;
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	ld	de, #0x000A
	add	hl, de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	((_sectorBuffer + 0x0016)), de
;fdisk2.c:165: strcpy(sector->params.standard.volumeLabelString, "NEXTOR 2.0 "); //it is same for DOS 2.20 format
	ld	hl,#__str_1
	ld	de,#(_sectorBuffer + 0x002b)
	xor	a, a
00117$:
	cp	a, (hl)
	ldi
	jr	NZ, 00117$
;fdisk2.c:166: sector->params.standard.serialNumber = GetNewSerialNumber(); //it is same for DOS 2.20 format
	call	_GetNewSerialNumber
	ld	c,l
	ld	b,h
	ld	((_sectorBuffer + 0x0027)), bc
	ld	((_sectorBuffer + 0x0027) + 2), de
;fdisk2.c:168: if(parameters->isFat16) {
	ld	c,-2 (ix)
	ld	b,-1 (ix)
	push	bc
	pop	iy
	ld	a,14 (iy)
;fdisk2.c:169: sector->params.standard.bigSectorCount = parameters->totalSectors;
;fdisk2.c:171: strcpy(sector->params.standard.fatTypeString, "FAT16   ");
;fdisk2.c:168: if(parameters->isFat16) {
	ld	-3 (ix), a
	or	a, a
	jr	Z,00104$
;fdisk2.c:169: sector->params.standard.bigSectorCount = parameters->totalSectors;
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	hl
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	ld	((_sectorBuffer + 0x0020)), de
	ld	((_sectorBuffer + 0x0020) + 2), bc
;fdisk2.c:170: sector->params.standard.extendedBlockSignature = 0x29;
	ld	hl,#(_sectorBuffer + 0x001c) + 10
	ld	(hl),#0x29
;fdisk2.c:171: strcpy(sector->params.standard.fatTypeString, "FAT16   ");
	ld	hl,#__str_2
	ld	de,#(_sectorBuffer + 0x0036)
	xor	a, a
00118$:
	cp	a, (hl)
	ldi
	jr	NZ, 00118$
	jr	00106$
00104$:
;fdisk2.c:173: sector->params.DOS220.z80JumpInstruction[0] = 0x18;
	ld	hl,#(_sectorBuffer + 0x001e)
	ld	(hl),#0x18
;fdisk2.c:174: sector->params.DOS220.z80JumpInstruction[1] = 0x1E;
	ld	hl,#(_sectorBuffer + 0x001f)
	ld	(hl),#0x1E
;fdisk2.c:175: strcpy(sector->params.DOS220.volIdString, "VOL_ID");
	ld	de,#(_sectorBuffer + 0x0020)
	ld	hl,#__str_3
	xor	a, a
00119$:
	cp	a, (hl)
	ldi
	jr	NZ, 00119$
;fdisk2.c:176: strcpy(sector->params.DOS220.fatTypeString, "FAT12   ");
	ld	de,#(_sectorBuffer + 0x0036)
	ld	hl,#__str_4
	xor	a, a
00120$:
	cp	a, (hl)
	ldi
	jr	NZ, 00120$
;fdisk2.c:177: memcpy(&(sector->params.DOS220.z80BootCode), SectorBootCode, (uint)0xC090 - (uint)0xC03E);
	ld	-2 (ix),#<(_SectorBootCode)
	ld	-1 (ix),#>(_SectorBootCode)
	ld	de,#(_sectorBuffer + 0x003e)
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	ld	bc,#0x0052
	ldir
00106$:
	ld	sp,ix
	pop	ix
	ret
_CreateFatBootSector_end::
__str_0:
	.ascii "NEXTOR20"
	.db 0x00
__str_1:
	.ascii "NEXTOR 2.0 "
	.db 0x00
__str_2:
	.ascii "FAT16   "
	.db 0x00
__str_3:
	.ascii "VOL_ID"
	.db 0x00
__str_4:
	.ascii "FAT12   "
	.db 0x00
;fdisk2.c:183: ulong GetNewSerialNumber() __naked
;	---------------------------------
; Function GetNewSerialNumber
; ---------------------------------
_GetNewSerialNumber_start::
_GetNewSerialNumber:
;fdisk2.c:223: __endasm;
	ld a,r
	xor b
	ld e,a
	or #128
	ld b,a
	gnsn_1:
	nop
	djnz gnsn_1
	ld a,r
	xor e
	ld d,a
	or #64
	ld b,a
	gnsn_2:
	nop
	nop
	djnz gnsn_2
	ld a,r
	xor d
	ld l,a
	or #32
	ld b,a
	gnsn_3:
	nop
	nop
	nop
	djnz gnsn_3
	ld a,r
	xor l
	ld h,a
	ret
_GetNewSerialNumber_end::
;fdisk2.c:227: void ClearSectorBuffer() __naked
;	---------------------------------
; Function ClearSectorBuffer
; ---------------------------------
_ClearSectorBuffer_start::
_ClearSectorBuffer:
;fdisk2.c:239: __endasm;
	ld hl,#_sectorBuffer
	ld de,#_sectorBuffer
	inc de
	ld bc,#512-1
	ld (hl),#0
	ldir
	ret
_ClearSectorBuffer_end::
;fdisk2.c:243: void SectorBootCode() __naked
;	---------------------------------
; Function SectorBootCode
; ---------------------------------
_SectorBootCode_start::
_SectorBootCode:
;fdisk2.c:283: __endasm;
	ret nc
	ld (#0xc07b),de
	ld de,#0xc078
	ld (hl),e
	inc hl
	ld (hl),d
	ld de,#0xc080
	ld c,#0x0f
	call #0xf37d
	inc a
	jp z,#0x4022
	ld de,#0x100
	ld c,#0x1a
	call #0xf37d
	ld hl,#1
	ld (#0xc08e),hl
	ld hl,#0x3f00
	ld de,#0xc080
	ld c,#0x27
	push de
	call #0xf37d
	pop de
	ld c,#0x10
	call #0xf37d
	jp 0x0100
	ld l,b
	ret nz
	call #0
	jp 0x4022
	nop
	.ascii "MSXDOS  SYS"
	nop
	nop
	nop
	nop
_SectorBootCode_end::
;fdisk2.c:287: int remote_CalculateFatFileSystemParameters(byte* callerParameters)
;	---------------------------------
; Function remote_CalculateFatFileSystemParameters
; ---------------------------------
_remote_CalculateFatFileSystemParameters_start::
_remote_CalculateFatFileSystemParameters:
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
	push	af
;fdisk2.c:289: ulong fileSystemSizeInK = *((ulong*)&callerParameters[0]);
	ld	c,4 (ix)
	ld	b,5 (ix)
	ld	e, c
	ld	d, b
	push	bc
	ld	hl, #0x0002
	add	hl, sp
	ex	de, hl
	ld	bc, #0x0004
	ldir
	pop	bc
;fdisk2.c:290: dosFilesystemParameters* parameters = *((dosFilesystemParameters**)&callerParameters[4]);
	ld	hl,#0x0004
	add	hl,bc
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
;fdisk2.c:291: CalculateFatFileSystemParameters(fileSystemSizeInK, parameters);
	push	de
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	push	hl
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	call	_CalculateFatFileSystemParameters
	ld	hl,#0x0006
	add	hl,sp
	ld	sp,hl
;fdisk2.c:292: return 0xB5;
	ld	hl,#0x00B5
	ld	sp,ix
	pop	ix
	ret
_remote_CalculateFatFileSystemParameters_end::
;fdisk2.c:296: void CalculateFatFileSystemParameters(ulong fileSystemSizeInK, dosFilesystemParameters* parameters)
;	---------------------------------
; Function CalculateFatFileSystemParameters
; ---------------------------------
_CalculateFatFileSystemParameters_start::
_CalculateFatFileSystemParameters:
;fdisk2.c:298: if(fileSystemSizeInK > MAX_FAT12_PARTITION_SIZE_IN_K) {
	xor	a, a
	ld	iy,#2
	add	iy,sp
	cp	a, 0 (iy)
	ld	a,#0x80
	sbc	a, 1 (iy)
	ld	a,#0x00
	sbc	a, 2 (iy)
	ld	a,#0x00
	sbc	a, 3 (iy)
	jr	NC,00102$
;fdisk2.c:299: CalculateFatFileSystemParametersFat16(fileSystemSizeInK, parameters);
	ld	hl, #6
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	push	bc
	ld	iy,#4
	add	iy,sp
	ld	l,2 (iy)
	ld	h,3 (iy)
	push	hl
	ld	l,0 (iy)
	ld	h,1 (iy)
	push	hl
	call	_CalculateFatFileSystemParametersFat16
	ld	hl,#0x0006
	add	hl,sp
	ld	sp,hl
	ret
00102$:
;fdisk2.c:301: CalculateFatFileSystemParametersFat12(fileSystemSizeInK, parameters);
	ld	hl, #6
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	push	bc
	ld	iy,#4
	add	iy,sp
	ld	l,2 (iy)
	ld	h,3 (iy)
	push	hl
	ld	l,0 (iy)
	ld	h,1 (iy)
	push	hl
	call	_CalculateFatFileSystemParametersFat12
	ld	hl,#0x0006
	add	hl,sp
	ld	sp,hl
	ret
_CalculateFatFileSystemParameters_end::
;fdisk2.c:306: int CalculateFatFileSystemParametersFat12(ulong fileSystemSizeInK, dosFilesystemParameters* parameters)
;	---------------------------------
; Function CalculateFatFileSystemParametersFat12
; ---------------------------------
_CalculateFatFileSystemParametersFat12_start::
_CalculateFatFileSystemParametersFat12:
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-21
	add	hl,sp
	ld	sp,hl
;fdisk2.c:317: uint maxClusterCount = MAX_FAT12_CLUSTER_COUNT;
	ld	-19 (ix),#0xF6
	ld	-18 (ix),#0x0F
;fdisk2.c:318: uint maxSectorsPerFat = 12;
	ld	hl,#0x000C
	ex	(sp), hl
;fdisk2.c:329: } else if(fileSystemSizeInK <= (16 * (ulong)1024)) {
	xor	a, a
	cp	a, 4 (ix)
	ld	a,#0x40
	sbc	a, 5 (ix)
	ld	a,#0x00
	sbc	a, 6 (ix)
	ld	a,#0x00
	sbc	a, 7 (ix)
	ld	a,#0x00
	rla
	ld	-1 (ix),a
;fdisk2.c:320: if(fileSystemSizeInK <= (2 * (ulong)1024)) {
	xor	a, a
	cp	a, 4 (ix)
	ld	a,#0x08
	sbc	a, 5 (ix)
	ld	a,#0x00
	sbc	a, 6 (ix)
	ld	a,#0x00
	sbc	a, 7 (ix)
	jr	C,00111$
;fdisk2.c:321: sectorsPerClusterPower = 0;
	ld	-17 (ix),#0x00
	ld	-16 (ix),#0x00
;fdisk2.c:322: sectorsPerCluster = 1;
	ld	-7 (ix),#0x01
	ld	-6 (ix),#0x00
	jr	00112$
00111$:
;fdisk2.c:323: } else if(fileSystemSizeInK <= (4 * (ulong)1024)) {
	xor	a, a
	cp	a, 4 (ix)
	ld	a,#0x10
	sbc	a, 5 (ix)
	ld	a,#0x00
	sbc	a, 6 (ix)
	ld	a,#0x00
	sbc	a, 7 (ix)
	jr	C,00108$
;fdisk2.c:324: sectorsPerClusterPower = 1;
	ld	-17 (ix),#0x01
	ld	-16 (ix),#0x00
;fdisk2.c:325: sectorsPerCluster = 2;
	ld	-7 (ix),#0x02
	ld	-6 (ix),#0x00
	jr	00112$
00108$:
;fdisk2.c:326: } else if(fileSystemSizeInK <= (8 * (ulong)1024)) {
	xor	a, a
	cp	a, 4 (ix)
	ld	a,#0x20
	sbc	a, 5 (ix)
	ld	a,#0x00
	sbc	a, 6 (ix)
	ld	a,#0x00
	sbc	a, 7 (ix)
	jr	C,00105$
;fdisk2.c:327: sectorsPerClusterPower = 2;
	ld	-17 (ix),#0x02
	ld	-16 (ix),#0x00
;fdisk2.c:328: sectorsPerCluster = 4;
	ld	-7 (ix),#0x04
	ld	-6 (ix),#0x00
	jr	00112$
00105$:
;fdisk2.c:329: } else if(fileSystemSizeInK <= (16 * (ulong)1024)) {
	ld	a,-1 (ix)
	or	a, a
	jr	NZ,00102$
;fdisk2.c:330: sectorsPerClusterPower = 3;
	ld	-17 (ix),#0x03
	ld	-16 (ix),#0x00
;fdisk2.c:331: sectorsPerCluster = 8;
	ld	-7 (ix),#0x08
	ld	-6 (ix),#0x00
	jr	00112$
00102$:
;fdisk2.c:333: sectorsPerClusterPower = 4;
	ld	-17 (ix),#0x04
	ld	-16 (ix),#0x00
;fdisk2.c:334: sectorsPerCluster = 16;
	ld	-7 (ix),#0x10
	ld	-6 (ix),#0x00
00112$:
;fdisk2.c:337: if(fileSystemSizeInK <= (16 * (ulong)1024)) {
	ld	a,-1 (ix)
	or	a, a
	jr	NZ,00114$
;fdisk2.c:338: maxClusterCount = 1021;
	ld	-19 (ix),#0xFD
	ld	-18 (ix),#0x03
;fdisk2.c:339: maxSectorsPerFat = 3;
	ld	hl,#0x0003
	ex	(sp), hl
;fdisk2.c:340: sectorsPerCluster *= 4;
	ld	a,#0x02+1
	jr	00144$
00143$:
	sla	-7 (ix)
	rl	-6 (ix)
00144$:
	dec	a
	jr	NZ,00143$
;fdisk2.c:341: sectorsPerClusterPower += 2;
	ld	a,-17 (ix)
	add	a, #0x02
	ld	-17 (ix),a
	ld	a,-16 (ix)
	adc	a, #0x00
	ld	-16 (ix),a
00114$:
;fdisk2.c:344: dataSectorsCount = (fileSystemSizeInK * 2) - (FAT12_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR) - 1;
	push	af
	ld	l,4 (ix)
	ld	h,5 (ix)
	ld	e,6 (ix)
	ld	d,7 (ix)
	pop	af
	add	hl, hl
	rl	e
	rl	d
	ld	a,l
	add	a,#0xF8
	ld	-15 (ix),a
	ld	a,h
	adc	a,#0xFF
	ld	-14 (ix),a
	ld	a,e
	adc	a,#0xFF
	ld	-13 (ix),a
	ld	a,d
	adc	a,#0xFF
	ld	-12 (ix),a
;fdisk2.c:346: clusterCount = dataSectorsCount >> sectorsPerClusterPower;
	ld	b,-17 (ix)
	push	af
	ld	l,-15 (ix)
	ld	h,-14 (ix)
	ld	e,-13 (ix)
	ld	d,-12 (ix)
	pop	af
	inc	b
	jr	00148$
00147$:
	srl	d
	rr	e
	rr	h
	rr	l
00148$:
	djnz	00147$
	ld	-11 (ix),l
	ld	-10 (ix),h
;fdisk2.c:347: sectorsPerFat = ((uint)clusterCount + 2) * 3;
	ld	l,-11 (ix)
	ld	h,-10 (ix)
	inc	hl
	inc	hl
	ld	c, l
	ld	b, h
	add	hl, hl
	add	hl, bc
	ld	-9 (ix),l
;fdisk2.c:350: sectorsPerFat >>= 10;
	ld	-8 (ix), h
	ld	a, h
	rrca
	rrca
	and	a,#0x3F
	ld	h,a
	ld	l,#0x00
;fdisk2.c:349: if((sectorsPerFat & 0x3FF) == 0) {
	ld	a,-9 (ix)
	or	a, a
	jr	NZ,00116$
	ld	a,-8 (ix)
	and	a, #0x03
	jr	NZ,00116$
;fdisk2.c:350: sectorsPerFat >>= 10;
	ld	-9 (ix),h
	ld	-8 (ix),l
	jr	00117$
00116$:
;fdisk2.c:352: sectorsPerFat >>= 10;
	ld	-9 (ix),h
	ld	-8 (ix),l
;fdisk2.c:353: sectorsPerFat++;
	inc	-9 (ix)
	jr	NZ,00151$
	inc	-8 (ix)
00151$:
00117$:
;fdisk2.c:356: clusterCount = (dataSectorsCount - FAT_COPIES * sectorsPerFat) >> sectorsPerClusterPower;
	ld	l,-9 (ix)
	ld	h,-8 (ix)
	add	hl, hl
	ld	e,#0x00
	ld	b,#0x00
	ld	a,-15 (ix)
	sub	a, l
	ld	l,a
	ld	a,-14 (ix)
	sbc	a, h
	ld	d,a
	ld	a,-13 (ix)
	sbc	a, e
	ld	e,a
	ld	a,-12 (ix)
	sbc	a, b
	ld	h,a
	ld	b,-17 (ix)
	inc	b
	jr	00153$
00152$:
	srl	h
	rr	e
	rr	d
	rr	l
00153$:
	djnz	00152$
	ld	-11 (ix),l
	ld	-10 (ix),d
;fdisk2.c:357: dataSectorsCount = (uint)clusterCount * (uint)sectorsPerCluster;
	ld	l,-7 (ix)
	ld	h,-6 (ix)
	push	hl
	ld	l,-11 (ix)
	ld	h,-10 (ix)
	push	hl
	call	__mulint_rrx_s
	pop	af
	pop	af
	ld	-15 (ix),l
	ld	-14 (ix),h
	ld	-13 (ix),#0x00
	ld	-12 (ix),#0x00
;fdisk2.c:359: if(clusterCount > maxClusterCount) {
	ld	a,-19 (ix)
	sub	a, -11 (ix)
	ld	a,-18 (ix)
	sbc	a, -10 (ix)
	jr	NC,00119$
;fdisk2.c:360: difference = clusterCount - maxClusterCount;
	ld	a,-11 (ix)
	sub	a, -19 (ix)
	ld	l,a
	ld	a,-10 (ix)
	sbc	a, -18 (ix)
	ld	h,a
;fdisk2.c:361: clusterCount = maxClusterCount;
	ld	a,-19 (ix)
	ld	-11 (ix),a
	ld	a,-18 (ix)
	ld	-10 (ix),a
;fdisk2.c:362: sectorsPerFat = maxSectorsPerFat;
	ld	a,-21 (ix)
	ld	-9 (ix),a
	ld	a,-20 (ix)
	ld	-8 (ix),a
;fdisk2.c:363: dataSectorsCount -= difference * sectorsPerCluster;
	ld	c,-7 (ix)
	ld	b,-6 (ix)
	push	bc
	push	hl
	call	__mulint_rrx_s
	pop	af
	pop	af
	ld	de,#0x0000
	ld	a,-15 (ix)
	sub	a, l
	ld	-15 (ix),a
	ld	a,-14 (ix)
	sbc	a, h
	ld	-14 (ix),a
	ld	a,-13 (ix)
	sbc	a, e
	ld	-13 (ix),a
	ld	a,-12 (ix)
	sbc	a, d
	ld	-12 (ix),a
00119$:
;fdisk2.c:366: parameters->totalSectors = dataSectorsCount + 1 + (sectorsPerFat * FAT_COPIES) + (FAT12_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	ld	e,8 (ix)
	ld	d,9 (ix)
	ld	l,-9 (ix)
	ld	h,-8 (ix)
	add	hl, hl
	ld	bc,#0x0008
	add	hl,bc
	ld	bc,#0x0000
	ld	a,-15 (ix)
	add	a, l
	ld	-5 (ix),a
	ld	a,-14 (ix)
	adc	a, h
	ld	-4 (ix),a
	ld	a,-13 (ix)
	adc	a, c
	ld	-3 (ix),a
	ld	a,-12 (ix)
	adc	a, b
	ld	-2 (ix),a
	push	de
	ld	hl, #0x0012
	add	hl, sp
	ld	bc, #0x0004
	ldir
	pop	de
;fdisk2.c:367: parameters->dataSectors = dataSectorsCount;
	ld	hl,#0x0004
	add	hl,de
	push	de
	ex	de,hl
	ld	hl, #0x0008
	add	hl, sp
	ld	bc, #0x0004
	ldir
	pop	de
;fdisk2.c:368: parameters->clusterCount = clusterCount;
	ld	hl,#0x0008
	add	hl,de
	ld	a,-11 (ix)
	ld	(hl),a
	inc	hl
	ld	a,-10 (ix)
	ld	(hl),a
;fdisk2.c:369: parameters->sectorsPerFat = sectorsPerFat;
	ld	hl,#0x000A
	add	hl,de
	ld	a,-9 (ix)
	ld	(hl),a
	inc	hl
	ld	a,-8 (ix)
	ld	(hl),a
;fdisk2.c:370: parameters->sectorsPerCluster = sectorsPerCluster;
	ld	hl,#0x000C
	add	hl,de
	ld	a,-7 (ix)
	ld	(hl),a
;fdisk2.c:371: parameters->sectorsPerRootDirectory = (FAT12_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	ld	hl,#0x000D
	add	hl,de
	ld	(hl),#0x07
;fdisk2.c:372: parameters->isFat16 = false;
	ld	hl,#0x000E
	add	hl,de
	ld	(hl),#0x00
;fdisk2.c:374: return 0;
	ld	hl,#0x0000
	ld	sp,ix
	pop	ix
	ret
_CalculateFatFileSystemParametersFat12_end::
;fdisk2.c:378: int CalculateFatFileSystemParametersFat16(ulong fileSystemSizeInK, dosFilesystemParameters* parameters)
;	---------------------------------
; Function CalculateFatFileSystemParametersFat16
; ---------------------------------
_CalculateFatFileSystemParametersFat16_start::
_CalculateFatFileSystemParametersFat16:
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-19
	add	hl,sp
	ld	sp,hl
;fdisk2.c:385: ulong fileSystemSizeInM = fileSystemSizeInK >> 10;
	push	af
	ld	a,4 (ix)
	ld	-19 (ix),a
	ld	a,5 (ix)
	ld	-18 (ix),a
	ld	a,6 (ix)
	ld	-17 (ix),a
	ld	a,7 (ix)
	ld	-16 (ix),a
	pop	af
	ld	b,#0x0A
00144$:
	srl	-16 (ix)
	rr	-17 (ix)
	rr	-18 (ix)
	rr	-19 (ix)
	djnz	00144$
;fdisk2.c:388: if(fileSystemSizeInM <= (ulong)128) {
	ld	a,#0x80
	cp	a, -19 (ix)
	ld	a,#0x00
	sbc	a, -18 (ix)
	ld	a,#0x00
	sbc	a, -17 (ix)
	ld	a,#0x00
	sbc	a, -16 (ix)
	jr	C,00114$
;fdisk2.c:389: sectorsPerClusterPower = 2;
	ld	bc,#0x0002
;fdisk2.c:390: sectorsPerCluster = 4;
	ld	-5 (ix),#0x04
	jp	00115$
00114$:
;fdisk2.c:391: } else if(fileSystemSizeInM <= (ulong)256) {
	xor	a, a
	cp	a, -19 (ix)
	ld	a,#0x01
	sbc	a, -18 (ix)
	ld	a,#0x00
	sbc	a, -17 (ix)
	ld	a,#0x00
	sbc	a, -16 (ix)
	jr	C,00111$
;fdisk2.c:392: sectorsPerClusterPower = 3;
	ld	bc,#0x0003
;fdisk2.c:393: sectorsPerCluster = 8;
	ld	-5 (ix),#0x08
	jr	00115$
00111$:
;fdisk2.c:394: } else if(fileSystemSizeInM <= (ulong)512) {
	xor	a, a
	cp	a, -19 (ix)
	ld	a,#0x02
	sbc	a, -18 (ix)
	ld	a,#0x00
	sbc	a, -17 (ix)
	ld	a,#0x00
	sbc	a, -16 (ix)
	jr	C,00108$
;fdisk2.c:395: sectorsPerClusterPower = 4;
	ld	bc,#0x0004
;fdisk2.c:396: sectorsPerCluster = 16;
	ld	-5 (ix),#0x10
	jr	00115$
00108$:
;fdisk2.c:397: } else if(fileSystemSizeInM <= (ulong)1024) {
	xor	a, a
	cp	a, -19 (ix)
	ld	a,#0x04
	sbc	a, -18 (ix)
	ld	a,#0x00
	sbc	a, -17 (ix)
	ld	a,#0x00
	sbc	a, -16 (ix)
	jr	C,00105$
;fdisk2.c:398: sectorsPerClusterPower = 5;
	ld	bc,#0x0005
;fdisk2.c:399: sectorsPerCluster = 32;
	ld	-5 (ix),#0x20
	jr	00115$
00105$:
;fdisk2.c:400: } else if(fileSystemSizeInM <= (ulong)2048) {
	xor	a, a
	cp	a, -19 (ix)
	ld	a,#0x08
	sbc	a, -18 (ix)
	ld	a,#0x00
	sbc	a, -17 (ix)
	ld	a,#0x00
	sbc	a, -16 (ix)
	jr	C,00102$
;fdisk2.c:401: sectorsPerClusterPower = 6;
	ld	bc,#0x0006
;fdisk2.c:402: sectorsPerCluster = 64;
	ld	-5 (ix),#0x40
	jr	00115$
00102$:
;fdisk2.c:404: sectorsPerClusterPower = 7;
	ld	bc,#0x0007
;fdisk2.c:405: sectorsPerCluster = 128;
	ld	-5 (ix),#0x80
00115$:
;fdisk2.c:408: dataSectorsCount = (fileSystemSizeInK * 2) - (FAT16_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR) - 1;
	push	af
	ld	l,4 (ix)
	ld	h,5 (ix)
	ld	e,6 (ix)
	ld	d,7 (ix)
	pop	af
	add	hl, hl
	rl	e
	rl	d
	ld	a,l
	add	a,#0xDF
	ld	-15 (ix),a
	ld	a,h
	adc	a,#0xFF
	ld	-14 (ix),a
	ld	a,e
	adc	a,#0xFF
	ld	-13 (ix),a
	ld	a,d
	adc	a,#0xFF
	ld	-12 (ix),a
;fdisk2.c:409: clusterCount = dataSectorsCount >> sectorsPerClusterPower;
	ld	a,c
	push	af
	ld	l,-15 (ix)
	ld	h,-14 (ix)
	ld	e,-13 (ix)
	ld	d,-12 (ix)
	pop	af
	inc	a
	jr	00149$
00148$:
	srl	d
	rr	e
	rr	h
	rr	l
00149$:
	dec	a
	jr	NZ,00148$
	ld	-11 (ix),l
	ld	-10 (ix),h
	ld	-9 (ix),e
	ld	-8 (ix),d
;fdisk2.c:410: sectorsPerFat = clusterCount + 2;
	ld	a,-11 (ix)
	add	a, #0x02
	ld	h,a
	ld	a,-10 (ix)
	adc	a, #0x00
	ld	d,a
	ld	a,-9 (ix)
	adc	a, #0x00
	ld	a,-8 (ix)
	adc	a, #0x00
	ld	-7 (ix),h
;fdisk2.c:413: sectorsPerFat >>= 8;
	ld	-6 (ix), d
	ld	h, d
	ld	l,#0x00
;fdisk2.c:412: if((sectorsPerFat & 0x3FF) == 0) {
	ld	a,-7 (ix)
	or	a, a
	jr	NZ,00117$
	ld	a,-6 (ix)
	and	a, #0x03
	jr	NZ,00117$
;fdisk2.c:413: sectorsPerFat >>= 8;
	ld	-7 (ix),h
	ld	-6 (ix),l
	jr	00118$
00117$:
;fdisk2.c:415: sectorsPerFat >>= 8;
	ld	-7 (ix),h
	ld	-6 (ix),l
;fdisk2.c:416: sectorsPerFat++;
	inc	-7 (ix)
	jr	NZ,00152$
	inc	-6 (ix)
00152$:
00118$:
;fdisk2.c:419: clusterCount = (dataSectorsCount - FAT_COPIES * sectorsPerFat);
	ld	l,-7 (ix)
	ld	h,-6 (ix)
	add	hl, hl
	ld	de,#0x0000
	ld	a,-15 (ix)
	sub	a, l
	ld	l,a
	ld	a,-14 (ix)
	sbc	a, h
	ld	h,a
	ld	a,-13 (ix)
	sbc	a, d
	ld	d,a
	ld	a,-12 (ix)
	sbc	a, e
	ld	e,a
	ld	-11 (ix),l
	ld	-10 (ix),h
	ld	-9 (ix),d
	ld	-8 (ix),e
;fdisk2.c:420: clusterCount >>= sectorsPerClusterPower;
	ld	a,c
	push	af
	pop	af
	inc	a
	jr	00154$
00153$:
	srl	-8 (ix)
	rr	-9 (ix)
	rr	-10 (ix)
	rr	-11 (ix)
00154$:
	dec	a
	jr	NZ,00153$
;fdisk2.c:421: dataSectorsCount = clusterCount << sectorsPerClusterPower;
	ld	a,c
	push	af
	ld	l,-11 (ix)
	ld	h,-10 (ix)
	ld	d,-9 (ix)
	ld	e,-8 (ix)
	pop	af
	inc	a
	jr	00156$
00155$:
	add	hl, hl
	rl	d
	rl	e
00156$:
	dec	a
	jr	NZ,00155$
	ld	-15 (ix),l
	ld	-14 (ix),h
	ld	-13 (ix),d
	ld	-12 (ix),e
;fdisk2.c:423: if(clusterCount > MAX_FAT16_CLUSTER_COUNT) {
	ld	a,#0xF6
	cp	a, -11 (ix)
	ld	a,#0xFF
	sbc	a, -10 (ix)
	ld	a,#0x00
	sbc	a, -9 (ix)
	ld	a,#0x00
	sbc	a, -8 (ix)
	jr	NC,00120$
;fdisk2.c:424: difference = clusterCount - MAX_FAT16_CLUSTER_COUNT;
	ld	a,-11 (ix)
	add	a,#0x0A
	ld	l,a
	ld	a,-10 (ix)
	adc	a,#0x00
	ld	h,a
	ld	a,-9 (ix)
	adc	a,#0xFF
	ld	e,a
	ld	a,-8 (ix)
	adc	a,#0xFF
	ld	d,a
;fdisk2.c:425: clusterCount = MAX_FAT16_CLUSTER_COUNT;
	ld	-11 (ix),#0xF6
	ld	-10 (ix),#0xFF
	ld	-9 (ix),#0x00
	ld	-8 (ix),#0x00
;fdisk2.c:426: sectorsPerFat = 256;
	ld	-7 (ix),#0x00
	ld	-6 (ix),#0x01
;fdisk2.c:427: dataSectorsCount -= difference << sectorsPerClusterPower;
	inc	c
	jr	00158$
00157$:
	add	hl, hl
	rl	e
	rl	d
00158$:
	dec	c
	jr	NZ,00157$
	ld	a,-15 (ix)
	sub	a, l
	ld	-15 (ix),a
	ld	a,-14 (ix)
	sbc	a, h
	ld	-14 (ix),a
	ld	a,-13 (ix)
	sbc	a, e
	ld	-13 (ix),a
	ld	a,-12 (ix)
	sbc	a, d
	ld	-12 (ix),a
00120$:
;fdisk2.c:430: parameters->totalSectors = dataSectorsCount + 1 + (sectorsPerFat * FAT_COPIES) + (FAT16_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	ld	e,8 (ix)
	ld	d,9 (ix)
	ld	l,-7 (ix)
	ld	h,-6 (ix)
	add	hl, hl
	ld	bc,#0x0021
	add	hl,bc
	ld	bc,#0x0000
	ld	a,-15 (ix)
	add	a, l
	ld	-4 (ix),a
	ld	a,-14 (ix)
	adc	a, h
	ld	-3 (ix),a
	ld	a,-13 (ix)
	adc	a, c
	ld	-2 (ix),a
	ld	a,-12 (ix)
	adc	a, b
	ld	-1 (ix),a
	push	de
	ld	hl, #0x0011
	add	hl, sp
	ld	bc, #0x0004
	ldir
	pop	de
;fdisk2.c:431: parameters->dataSectors = dataSectorsCount;
	ld	hl,#0x0004
	add	hl,de
	push	de
	ex	de,hl
	ld	hl, #0x0006
	add	hl, sp
	ld	bc, #0x0004
	ldir
	pop	de
;fdisk2.c:432: parameters->clusterCount = clusterCount;
	ld	hl,#0x0008
	add	hl,de
	ld	b,-11 (ix)
	ld	c,-10 (ix)
	ld	(hl),b
	inc	hl
	ld	(hl),c
;fdisk2.c:433: parameters->sectorsPerFat = sectorsPerFat;
	ld	hl,#0x000A
	add	hl,de
	ld	a,-7 (ix)
	ld	(hl),a
	inc	hl
	ld	a,-6 (ix)
	ld	(hl),a
;fdisk2.c:434: parameters->sectorsPerCluster = sectorsPerCluster;
	ld	hl,#0x000C
	add	hl,de
	ld	a,-5 (ix)
	ld	(hl),a
;fdisk2.c:435: parameters->sectorsPerRootDirectory = (FAT16_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	ld	hl,#0x000D
	add	hl,de
	ld	(hl),#0x20
;fdisk2.c:436: parameters->isFat16 = true;
	ld	hl,#0x000E
	add	hl,de
	ld	(hl),#0x01
;fdisk2.c:438: return 0;
	ld	hl,#0x0000
	ld	sp,ix
	pop	ix
	ret
_CalculateFatFileSystemParametersFat16_end::
;fdisk2.c:442: byte WriteSectorToDevice(byte driverSlot, byte deviceIndex, byte lunIndex, ulong firstDeviceSector)
;	---------------------------------
; Function WriteSectorToDevice
; ---------------------------------
_WriteSectorToDevice_start::
_WriteSectorToDevice:
	push	ix
	ld	ix,#0
	add	ix,sp
;fdisk2.c:444: regs.Flags.C = 1;
	ld	hl,#_regs
	ld	a,(hl)
	or	a,#0x01
	ld	(hl),a
;fdisk2.c:445: regs.Bytes.A = deviceIndex;
	ld	hl,#_regs + 1
	ld	a,5 (ix)
	ld	(hl),a
;fdisk2.c:446: regs.Bytes.B = 1;
	ld	hl,#_regs + 3
	ld	(hl),#0x01
;fdisk2.c:447: regs.Bytes.C = lunIndex;
	ld	hl,#_regs + 2
	ld	a,6 (ix)
	ld	(hl),a
;fdisk2.c:448: regs.Words.HL = (int)sectorBuffer;
	ld	de,#_sectorBuffer
	ld	((_regs + 0x0006)), de
;fdisk2.c:449: regs.Words.DE = (int)&firstDeviceSector;
	ld	hl,#0x0007
	add	hl,sp
	ex	de,hl
	ld	((_regs + 0x0004)), de
;fdisk2.c:451: DriverCall(driverSlot, DEV_RW);
	ld	hl,#0x4160
	push	hl
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_DriverCall
	pop	af
	inc	sp
;fdisk2.c:452: return regs.Bytes.A;
	ld	a, (#_regs + 1)
	ld	l,a
	pop	ix
	ret
_WriteSectorToDevice_end::
;fdisk2.c:456: int remote_PreparePartitionningProcess(byte* callerParameters)
;	---------------------------------
; Function remote_PreparePartitionningProcess
; ---------------------------------
_remote_PreparePartitionningProcess_start::
_remote_PreparePartitionningProcess:
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-6
	add	hl,sp
	ld	sp,hl
;fdisk2.c:462: driverSlot = callerParameters[0];
	ld	e,4 (ix)
	ld	d,5 (ix)
	ld	a,(de)
	ld	(#_driverSlot + 0),a
;fdisk2.c:463: deviceIndex = callerParameters[1];
	ld	l, e
	ld	h, d
	inc	hl
	ld	a,(hl)
	ld	(#_deviceIndex + 0),a
;fdisk2.c:464: selectedLunIndex = callerParameters[2];
	ld	l, e
	ld	h, d
	inc	hl
	inc	hl
	ld	a,(hl)
	ld	(#_selectedLunIndex + 0),a
;fdisk2.c:465: partitionsCount = *((uint*)&callerParameters[3]);
	ld	l, e
	ld	h, d
	inc	hl
	inc	hl
	inc	hl
	ld	a,(hl)
	ld	iy,#_partitionsCount
	ld	0 (iy),a
	inc	hl
	ld	a,(hl)
	ld	(#_partitionsCount + 1),a
;fdisk2.c:466: partitions = *((partitionInfo**)&callerParameters[5]);
	ld	hl,#0x0005
	add	hl,de
	ld	a,(hl)
	ld	iy,#_partitions
	ld	0 (iy),a
	inc	hl
	ld	a,(hl)
	ld	(#_partitions + 1),a
;fdisk2.c:467: sectorsPerTrack = *((uint*)&callerParameters[7]);
	ld	hl,#0x0007
	add	hl,de
	ld	a,(hl)
	ld	iy,#_sectorsPerTrack
	ld	0 (iy),a
	inc	hl
	ld	a,(hl)
	ld	(#_sectorsPerTrack + 1),a
;fdisk2.c:469: nextDeviceSector = 0;
	xor	a, a
	ld	(#_nextDeviceSector + 0),a
	ld	(#_nextDeviceSector + 1),a
	ld	(#_nextDeviceSector + 2),a
	ld	(#_nextDeviceSector + 3),a
;fdisk2.c:470: mainExtendedPartitionSectorCount = 0;
	xor	a, a
	ld	(#_mainExtendedPartitionSectorCount + 0),a
	ld	(#_mainExtendedPartitionSectorCount + 1),a
	ld	(#_mainExtendedPartitionSectorCount + 2),a
	ld	(#_mainExtendedPartitionSectorCount + 3),a
;fdisk2.c:471: mainExtendedPartitionFirstSector = 0;
	xor	a, a
	ld	(#_mainExtendedPartitionFirstSector + 0),a
	ld	(#_mainExtendedPartitionFirstSector + 1),a
	ld	(#_mainExtendedPartitionFirstSector + 2),a
	ld	iy,#_mainExtendedPartitionFirstSector
	ld	3 (iy),a
;fdisk2.c:473: for(i = 1; i < partitionsCount; i++) {
	ld	hl,#0x0001
	ex	(sp), hl
	ld	de,#0x0009
00103$:
	ld	hl,#_partitionsCount
	ld	a,-6 (ix)
	sub	a, (hl)
	ld	a,-5 (ix)
	inc	hl
	sbc	a, (hl)
	jp	PO, 00114$
	xor	a, #0x80
00114$:
	jp	P,00101$
;fdisk2.c:474: mainExtendedPartitionSectorCount += ((&partitions[i])->sizeInK * 2) + 1;	//+1 for the MBR
	ld	iy,(_partitions)
	add	iy, de
	ld	l,3 (iy)
	ld	h,4 (iy)
	ld	c,5 (iy)
	ld	b,6 (iy)
	add	hl, hl
	rl	c
	rl	b
	ld	a,l
	add	a, #0x01
	ld	-4 (ix),a
	ld	a,h
	adc	a, #0x00
	ld	-3 (ix),a
	ld	a,c
	adc	a, #0x00
	ld	-2 (ix),a
	ld	a,b
	adc	a, #0x00
	ld	-1 (ix),a
	ld	hl,#_mainExtendedPartitionSectorCount
	ld	a,(hl)
	add	a, -4 (ix)
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	adc	a, -3 (ix)
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	adc	a, -2 (ix)
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	adc	a, -1 (ix)
	ld	(hl),a
;fdisk2.c:473: for(i = 1; i < partitionsCount; i++) {
	ld	hl,#0x0009
	add	hl,de
	ex	de,hl
	inc	-6 (ix)
	jr	NZ,00103$
	inc	-5 (ix)
	jr	00103$
00101$:
;fdisk2.c:477: return 0;
	ld	hl,#0x0000
	ld	sp,ix
	pop	ix
	ret
_remote_PreparePartitionningProcess_end::
;fdisk2.c:481: int remote_CreatePartition(byte* callerParameters)
;	---------------------------------
; Function remote_CreatePartition
; ---------------------------------
_remote_CreatePartition_start::
_remote_CreatePartition:
;fdisk2.c:484: *((int*)&callerParameters[0]));
	pop	bc
	pop	hl
	push	hl
	push	bc
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	push	de
	call	_CreatePartition
	pop	af
	ret
_remote_CreatePartition_end::
;fdisk2.c:488: int CreatePartition(int index)
;	---------------------------------
; Function CreatePartition
; ---------------------------------
_CreatePartition_start::
_CreatePartition:
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-30
	add	hl,sp
	ld	sp,hl
;fdisk2.c:491: masterBootRecord* mbr = (masterBootRecord*)sectorBuffer;
;fdisk2.c:492: partitionInfo* partition = &partitions[index];
	ld	c,4 (ix)
	ld	b,5 (ix)
	ld	l, c
	ld	h, b
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, bc
	ld	e,l
	ld	d,h
	ld	iy,(_partitions)
	add	iy, de
	push	iy
	pop	af
	ld	-18 (ix),a
	push	iy
	dec	sp
	pop	af
	inc	sp
	ld	-19 (ix),a
;fdisk2.c:498: bool onlyPrimaryPartitions = (partitionsCount <= 4);
	ld	a,#0x04
	ld	iy,#_partitionsCount
	cp	a, 0 (iy)
	ld	a,#0x00
	ld	iy,#_partitionsCount
	sbc	a, 1 (iy)
	jp	PO, 00146$
	xor	a, #0x80
00146$:
	rlca
	and	a,#0x01
	xor	a,#0x01
	ld	d,a
;fdisk2.c:502: tableEntry = &(mbr->primaryPartitions[index]);
;fdisk2.c:500: if(onlyPrimaryPartitions) {
	ld	-30 (ix), d
	ld	a, d
	or	a, a
	jr	Z,00105$
;fdisk2.c:501: mbrSector = 0;
	xor	a, a
	ld	-23 (ix),a
	ld	-22 (ix),a
	ld	-21 (ix),a
	ld	-20 (ix),a
;fdisk2.c:502: tableEntry = &(mbr->primaryPartitions[index]);
	ld	l,4 (ix)
	ld	h,5 (ix)
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	ld	de,#(_sectorBuffer + 0x01be)
	add	hl,de
	ld	-29 (ix),l
	ld	-28 (ix),h
;fdisk2.c:503: if(index == 0) {
	ld	a,5 (ix)
	or	a,4 (ix)
	jr	NZ,00102$
;fdisk2.c:504: ClearSectorBuffer();
	call	_ClearSectorBuffer
;fdisk2.c:505: nextDeviceSector = 1;
	ld	hl,#_nextDeviceSector + 0
	ld	(hl), #0x01
	xor	a, a
	ld	(#_nextDeviceSector + 1),a
	ld	(#_nextDeviceSector + 2),a
	ld	(#_nextDeviceSector + 3),a
	jr	00103$
00102$:
;fdisk2.c:507: memcpy(sectorBuffer, sectorBufferBackup, 512);
	ld	hl,#_sectorBufferBackup
	ld	de,#_sectorBuffer
	ld	bc,#0x0200
	ldir
00103$:
;fdisk2.c:509: tableEntry->firstAbsoluteSector = nextDeviceSector;
	ld	a,-29 (ix)
	add	a, #0x08
	ld	l,a
	ld	a,-28 (ix)
	adc	a, #0x00
	ld	h,a
	ld	a,(#_nextDeviceSector + 0)
	ld	(hl),a
	inc	hl
	ld	a,(#_nextDeviceSector + 1)
	ld	(hl),a
	inc	hl
	ld	a,(#_nextDeviceSector + 2)
	ld	(hl),a
	inc	hl
	ld	iy,#_nextDeviceSector
	ld	a,3 (iy)
	ld	(hl),a
	jr	00106$
00105$:
;fdisk2.c:511: mbrSector = nextDeviceSector;
	ld	hl, #7
	add	hl, sp
	ex	de, hl
	ld	hl, #_nextDeviceSector
	ld	bc, #4
	ldir
;fdisk2.c:512: tableEntry = &(mbr->primaryPartitions[0]);
	ld	-29 (ix),#<((_sectorBuffer + 0x01be))
	ld	-28 (ix),#>((_sectorBuffer + 0x01be))
;fdisk2.c:513: ClearSectorBuffer();
	call	_ClearSectorBuffer
;fdisk2.c:514: tableEntry->firstAbsoluteSector = 1;
	ld	a,-29 (ix)
	add	a, #0x08
	ld	l,a
	ld	a,-28 (ix)
	adc	a, #0x00
	ld	h,a
	ld	(hl),#0x01
	inc	hl
	xor	a, a
	ld	(hl), a
	inc	hl
	ld	(hl), a
	inc	hl
	ld	(hl),#0x00
00106$:
;fdisk2.c:517: tableEntry->partitionType = partition->partitionType;
	ld	a,-29 (ix)
	add	a, #0x04
	ld	e,a
	ld	a,-28 (ix)
	adc	a, #0x00
	ld	d,a
	ld	l,-19 (ix)
	ld	h,-18 (ix)
	inc	hl
	inc	hl
	ld	a,(hl)
	ld	(de),a
;fdisk2.c:518: tableEntry->sectorCount = partition->sizeInK * 2;
	ld	a,-29 (ix)
	add	a, #0x0C
	ld	-2 (ix),a
	ld	a,-28 (ix)
	adc	a, #0x00
	ld	-1 (ix),a
	ld	a,-19 (ix)
	add	a, #0x03
	ld	-4 (ix),a
	ld	a,-18 (ix)
	adc	a, #0x00
	ld	-3 (ix),a
	ld	e,-4 (ix)
	ld	d,-3 (ix)
	ld	hl, #0x0016
	add	hl, sp
	ex	de, hl
	ld	bc, #0x0004
	ldir
	push	af
	pop	af
	sla	-8 (ix)
	rl	-7 (ix)
	rl	-6 (ix)
	rl	-5 (ix)
	ld	e,-2 (ix)
	ld	d,-1 (ix)
	ld	hl, #0x0016
	add	hl, sp
	ld	bc, #0x0004
	ldir
;fdisk2.c:520: firstFileSystemSector = mbrSector + tableEntry->firstAbsoluteSector;
	ld	a,-29 (ix)
	ld	-2 (ix),a
	ld	a,-28 (ix)
	ld	-1 (ix),a
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	ld	de, #0x0008
	add	hl, de
	ld	a,(hl)
	ld	-12 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-11 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-10 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-9 (ix),a
	ld	a,-23 (ix)
	add	a, -12 (ix)
	ld	-27 (ix),a
	ld	a,-22 (ix)
	adc	a, -11 (ix)
	ld	-26 (ix),a
	ld	a,-21 (ix)
	adc	a, -10 (ix)
	ld	-25 (ix),a
	ld	a,-20 (ix)
	adc	a, -9 (ix)
	ld	-24 (ix),a
;fdisk2.c:523: nextDeviceSector = tableEntry->firstAbsoluteSector + tableEntry->sectorCount;
	ld	a,-12 (ix)
	add	a, -8 (ix)
	ld	-12 (ix),a
	ld	a,-11 (ix)
	adc	a, -7 (ix)
	ld	-11 (ix),a
	ld	a,-10 (ix)
	adc	a, -6 (ix)
	ld	-10 (ix),a
	ld	a,-9 (ix)
	adc	a, -5 (ix)
	ld	-9 (ix),a
;fdisk2.c:522: if(onlyPrimaryPartitions){
	ld	a,-30 (ix)
	or	a, a
	jr	Z,00108$
;fdisk2.c:523: nextDeviceSector = tableEntry->firstAbsoluteSector + tableEntry->sectorCount;
	ld	de, #_nextDeviceSector
	ld	hl, #18
	add	hl, sp
	ld	bc, #4
	ldir
	jr	00109$
00108$:
;fdisk2.c:525: nextDeviceSector += tableEntry->firstAbsoluteSector + tableEntry->sectorCount;
	ld	hl,#_nextDeviceSector
	ld	a,(hl)
	add	a, -12 (ix)
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	adc	a, -11 (ix)
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	adc	a, -10 (ix)
	ld	(hl),a
	inc	hl
	ld	a,(hl)
	adc	a, -9 (ix)
	ld	(hl),a
00109$:
;fdisk2.c:528: if(!onlyPrimaryPartitions && index != (partitionsCount - 1)) {
	ld	a,-30 (ix)
	or	a, a
	jp	NZ,00114$
	ld	hl,#_partitionsCount + 0
	ld	e, (hl)
	ld	iy,#_partitionsCount
	ld	d,1 (iy)
	dec	de
	ld	a,4 (ix)
	sub	a, e
	jr	NZ,00149$
	ld	a,5 (ix)
	sub	a, d
	jp	Z,00114$
00149$:
;fdisk2.c:529: tableEntry++;
	ld	a,-29 (ix)
	add	a, #0x10
	ld	-29 (ix),a
	ld	a,-28 (ix)
	adc	a, #0x00
	ld	-28 (ix),a
;fdisk2.c:530: tableEntry->partitionType = PARTYPE_EXTENDED;
	ld	a,-29 (ix)
	add	a, #0x04
	ld	l,a
	ld	a,-28 (ix)
	adc	a, #0x00
	ld	h,a
	ld	(hl),#0x05
;fdisk2.c:531: tableEntry->firstAbsoluteSector = nextDeviceSector;
	ld	a,-29 (ix)
	add	a, #0x08
	ld	-12 (ix),a
	ld	a,-28 (ix)
	adc	a, #0x00
	ld	-11 (ix),a
	ld	l,-12 (ix)
	ld	h,-11 (ix)
	ld	a,(#_nextDeviceSector + 0)
	ld	(hl),a
	inc	hl
	ld	a,(#_nextDeviceSector + 1)
	ld	(hl),a
	inc	hl
	ld	a,(#_nextDeviceSector + 2)
	ld	(hl),a
	inc	hl
	ld	iy,#_nextDeviceSector
	ld	a,3 (iy)
	ld	(hl),a
;fdisk2.c:518: tableEntry->sectorCount = partition->sizeInK * 2;
	ld	a,-29 (ix)
	add	a, #0x0C
	ld	-8 (ix),a
	ld	a,-28 (ix)
	adc	a, #0x00
	ld	-7 (ix),a
;fdisk2.c:532: if(index == 0) {
	ld	a,5 (ix)
	or	a,4 (ix)
	jr	NZ,00111$
;fdisk2.c:533: mainExtendedPartitionFirstSector = nextDeviceSector;
	ld	de, #_mainExtendedPartitionFirstSector
	ld	hl, #_nextDeviceSector
	ld	bc, #4
	ldir
;fdisk2.c:534: tableEntry->sectorCount = mainExtendedPartitionSectorCount;
	ld	l,-8 (ix)
	ld	h,-7 (ix)
	ld	a,(#_mainExtendedPartitionSectorCount + 0)
	ld	(hl),a
	inc	hl
	ld	a,(#_mainExtendedPartitionSectorCount + 1)
	ld	(hl),a
	inc	hl
	ld	a,(#_mainExtendedPartitionSectorCount + 2)
	ld	(hl),a
	inc	hl
	ld	iy,#_mainExtendedPartitionSectorCount
	ld	a,3 (iy)
	ld	(hl),a
	jp	00114$
00111$:
;fdisk2.c:536: tableEntry->firstAbsoluteSector -= mainExtendedPartitionFirstSector;
	ld	l,-12 (ix)
	ld	h,-11 (ix)
	ld	d,(hl)
	inc	hl
	ld	e,(hl)
	inc	hl
	ld	b,(hl)
	inc	hl
	ld	c,(hl)
	ld	hl,#_mainExtendedPartitionFirstSector
	ld	a,d
	sub	a, (hl)
	ld	-16 (ix),a
	ld	a,e
	inc	hl
	sbc	a, (hl)
	ld	-15 (ix),a
	ld	a,b
	inc	hl
	sbc	a, (hl)
	ld	-14 (ix),a
	ld	a,c
	inc	hl
	sbc	a, (hl)
	ld	-13 (ix),a
	ld	e,-12 (ix)
	ld	d,-11 (ix)
	ld	hl, #0x000E
	add	hl, sp
	ld	bc, #0x0004
	ldir
;fdisk2.c:537: tableEntry->sectorCount = (((partitionInfo*)(partition + 1))->sizeInK * 2);
	ld	a,-19 (ix)
	add	a, #0x09
	ld	-16 (ix),a
	ld	a,-18 (ix)
	adc	a, #0x00
	ld	-15 (ix),a
	ld	l,-16 (ix)
	ld	h,-15 (ix)
	inc	hl
	inc	hl
	inc	hl
	ld	a,(hl)
	ld	-16 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-15 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-14 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-13 (ix),a
	push	af
	pop	af
	sla	-16 (ix)
	rl	-15 (ix)
	rl	-14 (ix)
	rl	-13 (ix)
	ld	e,-8 (ix)
	ld	d,-7 (ix)
	ld	hl, #0x000E
	add	hl, sp
	ld	bc, #0x0004
	ldir
00114$:
;fdisk2.c:541: if(index == 0) {
	ld	a,5 (ix)
	or	a,4 (ix)
	jr	NZ,00117$
;fdisk2.c:542: mbr->jumpInstruction[0] = 0xEB;
	ld	hl,#_sectorBuffer
	ld	(hl),#0xEB
;fdisk2.c:543: mbr->jumpInstruction[1] = 0xFE;
	inc	hl
	ld	(hl),#0xFE
;fdisk2.c:544: mbr->jumpInstruction[2] = 0x90;
	ld	hl,#_sectorBuffer + 2
	ld	(hl),#0x90
;fdisk2.c:545: strcpy(mbr->oemNameString, "NEXTOR20");
	ld	de,#(_sectorBuffer + 0x0003)
	ld	hl,#__str_5
	xor	a, a
00152$:
	cp	a, (hl)
	ldi
	jr	NZ, 00152$
00117$:
;fdisk2.c:548: mbr->mbrSignature = 0xAA55;
	ld	hl,#0xAA55
	ld	((_sectorBuffer + 0x01fe)), hl
;fdisk2.c:550: memcpy(sectorBufferBackup, sectorBuffer, 512);
	ld	de,#_sectorBufferBackup
	ld	hl,#_sectorBuffer
	ld	bc,#0x0200
	ldir
;fdisk2.c:552: if((error = WriteSectorToDevice(driverSlot, deviceIndex, selectedLunIndex, mbrSector)) != 0) {
	ld	l,-21 (ix)
	ld	h,-20 (ix)
	push	hl
	ld	l,-23 (ix)
	ld	h,-22 (ix)
	push	hl
	ld	a,(_selectedLunIndex)
	push	af
	inc	sp
	ld	a,(_deviceIndex)
	push	af
	inc	sp
	ld	a,(_driverSlot)
	push	af
	inc	sp
	call	_WriteSectorToDevice
	pop	af
	pop	af
	pop	af
	inc	sp
	ld	-17 (ix), l
	ld	-16 (ix), l
	ld	a,-17 (ix)
	or	a, a
	jr	Z,00119$
;fdisk2.c:553: return error;
	ld	l,-16 (ix)
	ld	h,#0x00
	jr	00120$
00119$:
;fdisk2.c:556: return CreateFatFileSystem(driverSlot, deviceIndex, selectedLunIndex, firstFileSystemSector, partition->sizeInK);
	ld	e,-4 (ix)
	ld	d,-3 (ix)
	ld	hl, #0x000E
	add	hl, sp
	ex	de, hl
	ld	bc, #0x0004
	ldir
	ld	l,-14 (ix)
	ld	h,-13 (ix)
	push	hl
	ld	l,-16 (ix)
	ld	h,-15 (ix)
	push	hl
	ld	l,-25 (ix)
	ld	h,-24 (ix)
	push	hl
	ld	l,-27 (ix)
	ld	h,-26 (ix)
	push	hl
	ld	a,(_selectedLunIndex)
	push	af
	inc	sp
	ld	a,(_deviceIndex)
	push	af
	inc	sp
	ld	a,(_driverSlot)
	push	af
	inc	sp
	call	_CreateFatFileSystem
	ld	iy,#0x000B
	add	iy,sp
	ld	sp,iy
	ld	-16 (ix), l
	ld	-16 (ix), l
	ld	-15 (ix),#0x00
	ld	l,-16 (ix)
	ld	h,-15 (ix)
00120$:
	ld	sp,ix
	pop	ix
	ret
_CreatePartition_end::
__str_5:
	.ascii "NEXTOR20"
	.db 0x00
;fdisk2.c:560: void putchar(char ch) __naked
;	---------------------------------
; Function putchar
; ---------------------------------
_putchar_start::
_putchar:
;fdisk2.c:570: __endasm;
	push ix
	ld ix,#4
	add ix,sp
	ld a,(ix)
	call 0x00A2
	pop ix
	ret
_putchar_end::
;fdisk2.c:574: void Locate(byte x, byte y)
;	---------------------------------
; Function Locate
; ---------------------------------
_Locate_start::
_Locate:
;fdisk2.c:576: regs.Bytes.H = x + 1;
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	inc	a
	ld	(#(_regs + 0x0007)),a
;fdisk2.c:577: regs.Bytes.L = y + 1;
	ld	hl, #3+0
	add	hl, sp
	ld	a, (hl)
	inc	a
	ld	(#(_regs + 0x0006)),a
;fdisk2.c:578: AsmCall(POSIT, &regs, REGS_MAIN, REGS_NONE);
	ld	de,#_regs
	ld	hl,#0x0000
	push	hl
	ld	l, #0x02
	push	hl
	push	de
	ld	l, #0xC6
	push	hl
	call	_AsmCallAlt
	ld	hl,#0x0008
	add	hl,sp
	ld	sp,hl
	ret
_Locate_end::
;asmcall.c:10: void DriverCall(byte slot, uint routineAddress)
;	---------------------------------
; Function DriverCall
; ---------------------------------
_DriverCall_start::
_DriverCall:
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-8
	add	hl,sp
	ld	sp,hl
;asmcall.c:15: memcpy(registerData, &regs, 8);
	ld	hl,#0x0000
	add	hl,sp
	ld	c,l
	ld	b,h
	ld	e, c
	ld	d, b
	ld	hl,#_regs
	push	bc
	ld	bc,#0x0008
	ldir
	pop	bc
;asmcall.c:17: regs.Bytes.A = slot;
	ld	hl,#(_regs + 0x0001)
	ld	a,4 (ix)
	ld	(hl),a
;asmcall.c:18: regs.Bytes.B = 0xFF;
	ld	hl,#_regs + 3
	ld	(hl),#0xFF
;asmcall.c:19: regs.UWords.DE = routineAddress;
	ld	hl,#_regs + 4
	ld	a,5 (ix)
	ld	(hl),a
	inc	hl
	ld	a,6 (ix)
	ld	(hl),a
;asmcall.c:20: regs.Words.HL = (int)registerData;
	ld	((_regs + 0x0006)), bc
;asmcall.c:22: DosCall(_CDRVR, REGS_ALL);
	ld	hl,#0x037B
	push	hl
	call	_DosCall
	pop	af
;asmcall.c:24: if(regs.Bytes.A == 0) {
	ld	a, (#(_regs + 0x0001) + 0)
	or	a, a
	jr	NZ,00103$
;asmcall.c:25: regs.Words.AF = regs.Words.IX;
	ld	de, (#_regs + 8)
	ld	(_regs), de
00103$:
	ld	sp,ix
	pop	ix
	ret
_DriverCall_end::
;asmcall.c:30: void DosCall(byte function, register_usage outRegistersDetail)
;	---------------------------------
; Function DosCall
; ---------------------------------
_DosCall_start::
_DosCall:
;asmcall.c:32: regs.Bytes.C = function;
	ld	hl,#_regs + 2
	ld	iy,#2
	add	iy,sp
	ld	a,0 (iy)
	ld	(hl),a
;asmcall.c:33: SwitchSystemBankThenCall(0xF37D, outRegistersDetail);
	ld	hl, #3+0
	add	hl, sp
	ld	a, (hl)
	push	af
	inc	sp
	ld	hl,#0xF37D
	push	hl
	call	_SwitchSystemBankThenCall
	pop	af
	inc	sp
	ret
_DosCall_end::
;asmcall.c:37: void SwitchSystemBankThenCall(int routineAddress, register_usage outRegistersDetail)
;	---------------------------------
; Function SwitchSystemBankThenCall
; ---------------------------------
_SwitchSystemBankThenCall_start::
_SwitchSystemBankThenCall:
;asmcall.c:39: *((int*)BK4_ADD) = routineAddress;
	ld	hl,#0xF84C
	ld	iy,#2
	add	iy,sp
	ld	a,0 (iy)
	ld	(hl),a
	inc	hl
	ld	a,1 (iy)
	ld	(hl),a
;asmcall.c:40: AsmCall(CALLB0, &regs, REGS_ALL, outRegistersDetail);
	ld	de,#_regs
	ld	hl,#0x0000
	push	hl
	ld	hl, #6+0
	add	hl, sp
	ld	b, (hl)
	ld	c,#0x03
	push	bc
	push	de
	ld	hl,#0x403F
	push	hl
	call	_AsmCallAlt
	ld	hl,#0x0008
	add	hl,sp
	ld	sp,hl
	ret
_SwitchSystemBankThenCall_end::
;asmcall.c:44: void AsmCallAlt(uint address, Z80_registers* regs, register_usage inRegistersDetail, register_usage outRegistersDetail, int alternateAf) __naked
;	---------------------------------
; Function AsmCallAlt
; ---------------------------------
_AsmCallAlt_start::
_AsmCallAlt:
;asmcall.c:157: __endasm;
	push ix
	ld ix,#4
	add ix,sp
	ld e,6(ix) ;Alternate AF
	ld d,7(ix)
	ex af,af
	push de
	pop af
	ex af,af
	ld l,(ix) ;HL=Routine address
	ld h,1(ix)
	ld e,2(ix) ;DE=regs address
	ld d,3(ix)
	ld a,5(ix)
	ld (_OUT_FLAGS),a
	ld a,4(ix) ;A=in registers detail
	ld (_ASMRUT+1),hl
	push de
	or a
	jr z,ASMRUT_DO
	push de
	pop ix ;IX=&Z80regs
	exx
	ld l,(ix)
	ld h,1(ix) ;AF
	dec a
	jr z,ASMRUT_DOAF
	exx
	ld c,2(ix) ;BC, DE, HL
	ld b,3(ix)
	ld e,4(ix)
	ld d,5(ix)
	ld l,6(ix)
	ld h,7(ix)
	dec a
	exx
	jr z,ASMRUT_DOAF
	ld c,8(ix) ;IX
	ld b,9(ix)
	ld e,10(ix) ;IY
	ld d,11(ix)
	push de
	push bc
	pop ix
	pop iy
	ASMRUT_DOAF:
	push hl
	pop af
	exx
	ASMRUT_DO:
	call _ASMRUT
;ASMRUT: call 0
	ex (sp),ix ;IX to stack, now IX=&Z80regs
	ex af,af ;Alternate AF
	ld a,(_OUT_FLAGS)
	or a
	jr z,CALL_END
	exx ;Alternate HLDEBC
	ex af,af ;Main AF
	push af
	pop hl
	ld (ix),l
	ld 1(ix),h
	exx ;Main HLDEBC
	ex af,af ;Alternate AF
	dec a
	jr z,CALL_END
	ld 2(ix),c ;BC, DE, HL
	ld 3(ix),b
	ld 4(ix),e
	ld 5(ix),d
	ld 6(ix),l
	ld 7(ix),h
	dec a
	jr z,CALL_END
	exx ;Alternate HLDEBC
	pop hl
	ld 8(ix),l ;IX
	ld 9(ix),h
	push iy
	pop hl
	ld 10(ix),l ;IY
	ld 11(ix),h
	exx ;Main HLDEBC
	ex af,af
	pop ix
	ret
	CALL_END:
	ex af,af
	pop hl
	pop ix
	ret
;OUT_FLAGS: .db #0
_AsmCallAlt_end::
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
