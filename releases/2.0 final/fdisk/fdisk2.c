//FDISK - Disk partitionner for Nextor
//This is the extra functions file.
//These functions are called from the main program (fidsk.c)
//by using its CallFunctionInExtraBank function. 

// SDCC compilation command line:
//
// sdcc --code-loc 0x4150 --data-loc 0xA000 -mz80 --disable-warning 196 --disable-warning 84 --no-std-crt0 fdisk_crt0.rel msxchar.lib asm.lib fdisk2.c
// hex2bin -e dat fdisk2.ihx
//
// Once compiled, embed the first 8000 bytes of fdisk.dat at position 98560 of the appropriate Nextor ROM file:
// dd if=fdisk2.dat of=nextor.rom bs=1 count=8000 seek=98560

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "asm.h"
#include "system.h"
#include "dos.h"
#include "types.h"
#include "partit.h"
#include "fdisk.h"
#include "asmcall.h"

byte sectorBuffer[512];
byte sectorBufferBackup[512];
byte ASMRUT[4];
byte OUT_FLAGS;
Z80_registers regs;
byte driverSlot;
byte deviceIndex;
byte selectedLunIndex;
int partitionsCount;
partitionInfo* partitions;
ulong nextDeviceSector;
ulong mainExtendedPartitionSectorCount;
ulong mainExtendedPartitionFirstSector;
uint sectorsPerTrack;

#define Clear(address, len) memset(address, 0, len)

int remote_CreateFatFileSystem(byte* callerParameters);
byte CreateFatFileSystem(byte driverSlot, byte deviceIndex, byte lunIndex, ulong firstDeviceSector, ulong fileSystemSizeInK);
void CreateFatBootSector(dosFilesystemParameters* parameters);
ulong GetNewSerialNumber();
void ClearSectorBuffer();
void SectorBootCode();
int remote_CalculateFatFileSystemParameters(byte* callerParameters);
void CalculateFatFileSystemParameters(ulong fileSystemSizeInK, dosFilesystemParameters* parameters);
int CalculateFatFileSystemParametersFat12(ulong fileSystemSizeInK, dosFilesystemParameters* parameters);
int CalculateFatFileSystemParametersFat16(ulong fileSystemSizeInK, dosFilesystemParameters* parameters);
byte WriteSectorToDevice(byte driverSlot, byte deviceIndex, byte lunIndex, ulong firstDeviceSector);
int remote_PreparePartitionningProcess(byte* callerParameters);
int remote_CreatePartition(byte* callerParameters);
int CreatePartition(int index);
void putchar(char ch);
void Locate(byte x, byte y);

//BC = function number (defined in fdisk.h), HL = address of parameters block
int main(int bc, int hl)
{
    ASMRUT[0] = 0xC3;   //Code for JP
	switch(bc) {
		case f_CalculateFatFileSystemParameters:
			return remote_CalculateFatFileSystemParameters((byte*)hl);
			break;
		case f_CreateFatFileSystem:
			return remote_CreateFatFileSystem((byte*)hl);
			break;
		case f_PreparePartitionningProcess:
			return remote_PreparePartitionningProcess((byte*)hl);
			break;
		case f_CreatePartition:
			return remote_CreatePartition((byte*)hl);
			break;
		default:
			return 0;
	}
}


int remote_CreateFatFileSystem(byte* callerParameters)
{
	return (int)CreateFatFileSystem(
		callerParameters[0],
		callerParameters[1],
		callerParameters[2],
		*((ulong*)&callerParameters[3]),
		*((ulong*)&callerParameters[7]));
}


byte CreateFatFileSystem(byte driverSlot, byte deviceIndex, byte lunIndex, ulong firstDeviceSector, ulong fileSystemSizeInK)
{
	dosFilesystemParameters parameters;
	byte error;
	ulong sectorNumber;
	uint zeroSectorsToWrite;
	uint i;

	CalculateFatFileSystemParameters(fileSystemSizeInK, &parameters);
	
	//* Boot sector

	CreateFatBootSector(&parameters);

	if((error = WriteSectorToDevice(driverSlot, deviceIndex, lunIndex, firstDeviceSector)) != 0) {
		return error;
	}

	//* FAT (except 1st sector) and root directory sectors

	ClearSectorBuffer();
	zeroSectorsToWrite = (parameters.sectorsPerFat * FAT_COPIES) + (parameters.sectorsPerRootDirectory) - 1;
	sectorNumber = firstDeviceSector + 2;
	for(i = 0; i < zeroSectorsToWrite; i++) {
		if((error = WriteSectorToDevice(driverSlot, deviceIndex, lunIndex, sectorNumber)) != 0) {
			return error;
		}
		sectorNumber++;
	}

	//* First sector of each FAT

	sectorBuffer[0] = 0xF0;
	sectorBuffer[1] = 0xFF;
	sectorBuffer[2] = 0xFF;
	if(parameters.isFat16) {
		sectorBuffer[3] = 0xFF;
	}
	if((error = WriteSectorToDevice(driverSlot, deviceIndex, lunIndex, firstDeviceSector + 1)) != 0) {
		return error;
	}
	if((error = WriteSectorToDevice(driverSlot, deviceIndex, lunIndex, firstDeviceSector + 1 + parameters.sectorsPerFat)) != 0) {
		return error;
	}

	//* Done

	return 0;
}


void CreateFatBootSector(dosFilesystemParameters* parameters)
{
	fatBootSector* sector = (fatBootSector*)sectorBuffer;

	ClearSectorBuffer();

	sector->jumpInstruction[0] = 0xEB;
	sector->jumpInstruction[1] = 0xFE;
	sector->jumpInstruction[2] = 0x90;
	strcpy(sector->oemNameString, "NEXTOR20");
	sector->sectorSize = 512;
	sector->sectorsPerCluster = parameters->sectorsPerCluster;
	sector->reservedSectors = 1;
	sector->numberOfFats = FAT_COPIES;
	sector->rootDirectoryEntries = parameters->sectorsPerRootDirectory * DIR_ENTRIES_PER_SECTOR;
	if((parameters->totalSectors & 0xFFFF0000) == 0) {
		sector->smallSectorCount = parameters->totalSectors;
	}
	sector->mediaId = 0xF0;
	sector->sectorsPerFat = parameters->sectorsPerFat;
	strcpy(sector->params.standard.volumeLabelString, "NEXTOR 2.0 "); //it is same for DOS 2.20 format
	sector->params.standard.serialNumber = GetNewSerialNumber(); //it is same for DOS 2.20 format

	if(parameters->isFat16) {
		sector->params.standard.bigSectorCount = parameters->totalSectors;
		sector->params.standard.extendedBlockSignature = 0x29;
		strcpy(sector->params.standard.fatTypeString, "FAT16   ");
	} else {
		sector->params.DOS220.z80JumpInstruction[0] = 0x18;
		sector->params.DOS220.z80JumpInstruction[1] = 0x1E;
		strcpy(sector->params.DOS220.volIdString, "VOL_ID");
		strcpy(sector->params.DOS220.fatTypeString, "FAT12   ");
		memcpy(&(sector->params.DOS220.z80BootCode), SectorBootCode, (uint)0xC090 - (uint)0xC03E);
	}
	
}


ulong GetNewSerialNumber() __naked
{
	__asm

		ld a,r
		xor	b
		ld e,a
		or #128
		ld b,a
gnsn_1:
		nop
		djnz gnsn_1

		ld	a,r
		xor	e
		ld d,a
		or #64
		ld b,a
gnsn_2:
		nop
		nop
		djnz gnsn_2

		ld	a,r
		xor	d
		ld l,a
		or #32
		ld b,a
gnsn_3:
		nop
		nop
		nop
		djnz gnsn_3

		ld	a,r
		xor l
		ld	h,a

		ret

	__endasm;
}


void ClearSectorBuffer() __naked
{
	__asm

		ld hl,#_sectorBuffer
		ld de,#_sectorBuffer
		inc de
		ld bc,#512-1
		ld (hl),#0
		ldir
		ret

	__endasm;
}


void SectorBootCode() __naked
{
	__asm

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

	__endasm;
}

//used only for debugging
int remote_CalculateFatFileSystemParameters(byte* callerParameters)
{
	ulong fileSystemSizeInK = *((ulong*)&callerParameters[0]);
	dosFilesystemParameters* parameters = *((dosFilesystemParameters**)&callerParameters[4]);
	CalculateFatFileSystemParameters(fileSystemSizeInK, parameters);
	return 0xB5;
}


void CalculateFatFileSystemParameters(ulong fileSystemSizeInK, dosFilesystemParameters* parameters)
{
	if(fileSystemSizeInK > MAX_FAT12_PARTITION_SIZE_IN_K) {
		CalculateFatFileSystemParametersFat16(fileSystemSizeInK, parameters);
	} else {
		CalculateFatFileSystemParametersFat12(fileSystemSizeInK, parameters);
	}
}


int CalculateFatFileSystemParametersFat12(ulong fileSystemSizeInK, dosFilesystemParameters* parameters)
{
	//Note: Partitions <=16M are defined to have at most 3 sectors per FAT,
	//so that they can boot DOS 1. This limits the cluster count to 1021.

	uint sectorsPerCluster;
	uint sectorsPerFat;
	uint clusterCount;
	ulong dataSectorsCount;
	uint difference;
	uint sectorsPerClusterPower;
	uint maxClusterCount = MAX_FAT12_CLUSTER_COUNT;
	uint maxSectorsPerFat = 12;

	if(fileSystemSizeInK <= (2 * (ulong)1024)) {
		sectorsPerClusterPower = 0;
		sectorsPerCluster = 1;
	} else if(fileSystemSizeInK <= (4 * (ulong)1024)) {
		sectorsPerClusterPower = 1;
		sectorsPerCluster = 2;
	} else if(fileSystemSizeInK <= (8 * (ulong)1024)) {
		sectorsPerClusterPower = 2;
		sectorsPerCluster = 4;
	} else if(fileSystemSizeInK <= (16 * (ulong)1024)) {
		sectorsPerClusterPower = 3;
		sectorsPerCluster = 8;
	} else {
		sectorsPerClusterPower = 4;
		sectorsPerCluster = 16;
	}

    if(fileSystemSizeInK <= (16 * (ulong)1024)) {
        maxClusterCount = 1021;
		maxSectorsPerFat = 3;
        sectorsPerCluster *= 4;
        sectorsPerClusterPower += 2;
    }

	dataSectorsCount = (fileSystemSizeInK * 2) - (FAT12_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR) - 1;

	clusterCount = dataSectorsCount >> sectorsPerClusterPower;
	sectorsPerFat = ((uint)clusterCount + 2) * 3;

	if((sectorsPerFat & 0x3FF) == 0) {
		sectorsPerFat >>= 10;
	} else {
		sectorsPerFat >>= 10;
		sectorsPerFat++;
	}
	
	clusterCount = (dataSectorsCount - FAT_COPIES * sectorsPerFat) >> sectorsPerClusterPower;
	dataSectorsCount = (uint)clusterCount * (uint)sectorsPerCluster;

	if(clusterCount > maxClusterCount) {
		difference = clusterCount - maxClusterCount;
		clusterCount = maxClusterCount;
		sectorsPerFat = maxSectorsPerFat;
		dataSectorsCount -= difference * sectorsPerCluster;
	}

	parameters->totalSectors = dataSectorsCount + 1 + (sectorsPerFat * FAT_COPIES) + (FAT12_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	parameters->dataSectors = dataSectorsCount;
	parameters->clusterCount = clusterCount;
	parameters->sectorsPerFat = sectorsPerFat;
	parameters->sectorsPerCluster = sectorsPerCluster;
	parameters->sectorsPerRootDirectory = (FAT12_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	parameters->isFat16 = false;

	return 0;
}


int CalculateFatFileSystemParametersFat16(ulong fileSystemSizeInK, dosFilesystemParameters* parameters)
{
	byte sectorsPerCluster;
	uint sectorsPerFat;
	ulong clusterCount;
	ulong dataSectorsCount;
	uint sectorsPerClusterPower;
	ulong fileSystemSizeInM = fileSystemSizeInK >> 10;
	ulong difference;

	if(fileSystemSizeInM <= (ulong)128) {
		sectorsPerClusterPower = 2;
		sectorsPerCluster = 4;
	} else if(fileSystemSizeInM <= (ulong)256) {
		sectorsPerClusterPower = 3;
		sectorsPerCluster = 8;
	} else if(fileSystemSizeInM <= (ulong)512) {
		sectorsPerClusterPower = 4;
		sectorsPerCluster = 16;
	} else if(fileSystemSizeInM <= (ulong)1024) {
		sectorsPerClusterPower = 5;
		sectorsPerCluster = 32;
	} else if(fileSystemSizeInM <= (ulong)2048) {
		sectorsPerClusterPower = 6;
		sectorsPerCluster = 64;
	} else {
		sectorsPerClusterPower = 7;
		sectorsPerCluster = 128;
	}

	dataSectorsCount = (fileSystemSizeInK * 2) - (FAT16_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR) - 1;
	clusterCount = dataSectorsCount >> sectorsPerClusterPower;
	sectorsPerFat = clusterCount + 2;

	if((sectorsPerFat & 0x3FF) == 0) {
		sectorsPerFat >>= 8;
	} else {
		sectorsPerFat >>= 8;
		sectorsPerFat++;
	}

	clusterCount = (dataSectorsCount - FAT_COPIES * sectorsPerFat);
	clusterCount >>= sectorsPerClusterPower;
    dataSectorsCount = clusterCount << sectorsPerClusterPower;

	if(clusterCount > MAX_FAT16_CLUSTER_COUNT) {
		difference = clusterCount - MAX_FAT16_CLUSTER_COUNT;
		clusterCount = MAX_FAT16_CLUSTER_COUNT;
		sectorsPerFat = 256;
		dataSectorsCount -= difference << sectorsPerClusterPower;
	}

	parameters->totalSectors = dataSectorsCount + 1 + (sectorsPerFat * FAT_COPIES) + (FAT16_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	parameters->dataSectors = dataSectorsCount;
	parameters->clusterCount = clusterCount;
	parameters->sectorsPerFat = sectorsPerFat;
	parameters->sectorsPerCluster = sectorsPerCluster;
	parameters->sectorsPerRootDirectory = (FAT16_ROOT_DIR_ENTRIES / DIR_ENTRIES_PER_SECTOR);
	parameters->isFat16 = true;

	return 0;
}


byte WriteSectorToDevice(byte driverSlot, byte deviceIndex, byte lunIndex, ulong firstDeviceSector)
{
	regs.Flags.C = 1;
	regs.Bytes.A = deviceIndex;
	regs.Bytes.B = 1;
	regs.Bytes.C = lunIndex;
	regs.Words.HL = (int)sectorBuffer;
	regs.Words.DE = (int)&firstDeviceSector;

	DriverCall(driverSlot, DEV_RW);
	return regs.Bytes.A;
}


int remote_PreparePartitionningProcess(byte* callerParameters)
{
	int i;
	int sectorsRemaining;
	partitionInfo* partition = &partitions[1];

	driverSlot = callerParameters[0];
	deviceIndex = callerParameters[1];
	selectedLunIndex = callerParameters[2];
	partitionsCount = *((uint*)&callerParameters[3]);
	partitions = *((partitionInfo**)&callerParameters[5]);
	sectorsPerTrack = *((uint*)&callerParameters[7]);

	nextDeviceSector = 0;
	mainExtendedPartitionSectorCount = 0;
	mainExtendedPartitionFirstSector = 0;

	for(i = 1; i < partitionsCount; i++) {
		mainExtendedPartitionSectorCount += ((&partitions[i])->sizeInK * 2) + 1;	//+1 for the MBR
	}

	return 0;
}


int remote_CreatePartition(byte* callerParameters)
{
	return (int)CreatePartition(
		*((int*)&callerParameters[0]));
}


int CreatePartition(int index)
{
	byte error;
	masterBootRecord* mbr = (masterBootRecord*)sectorBuffer;
	partitionInfo* partition = &partitions[index];
	ulong mbrSector;
	uint paddingSectors;
	ulong firstFileSystemSector;
	ulong extendedPartitionFirstAbsoluteSector;
	partitionTableEntry* tableEntry;
	bool onlyPrimaryPartitions = (partitionsCount <= 4);

	if(onlyPrimaryPartitions) {
		mbrSector = 0;
		tableEntry = &(mbr->primaryPartitions[index]);
		if(index == 0) {
			ClearSectorBuffer();
			nextDeviceSector = 1;
		} else {
			memcpy(sectorBuffer, sectorBufferBackup, 512);
		}
		tableEntry->firstAbsoluteSector = nextDeviceSector;
	} else {
		mbrSector = nextDeviceSector;
		tableEntry = &(mbr->primaryPartitions[0]);
		ClearSectorBuffer();
		tableEntry->firstAbsoluteSector = 1;
	}

	tableEntry->partitionType = partition->partitionType;
	tableEntry->sectorCount = partition->sizeInK * 2;

	firstFileSystemSector = mbrSector + tableEntry->firstAbsoluteSector;

	if(onlyPrimaryPartitions){
		nextDeviceSector = tableEntry->firstAbsoluteSector + tableEntry->sectorCount;
	} else {
		nextDeviceSector += tableEntry->firstAbsoluteSector + tableEntry->sectorCount;
	}

	if(!onlyPrimaryPartitions && index != (partitionsCount - 1)) {
		tableEntry++;
		tableEntry->partitionType = PARTYPE_EXTENDED;
		tableEntry->firstAbsoluteSector = nextDeviceSector;
		if(index == 0) {
			mainExtendedPartitionFirstSector = nextDeviceSector;
			tableEntry->sectorCount = mainExtendedPartitionSectorCount;
		} else {
			tableEntry->firstAbsoluteSector -= mainExtendedPartitionFirstSector;
			tableEntry->sectorCount = (((partitionInfo*)(partition + 1))->sizeInK * 2);
		}
	}

    if(index == 0) {
        mbr->jumpInstruction[0] = 0xEB;
    	mbr->jumpInstruction[1] = 0xFE;
    	mbr->jumpInstruction[2] = 0x90;
    	strcpy(mbr->oemNameString, "NEXTOR20");
    }

	mbr->mbrSignature = 0xAA55;

	memcpy(sectorBufferBackup, sectorBuffer, 512);

	if((error = WriteSectorToDevice(driverSlot, deviceIndex, selectedLunIndex, mbrSector)) != 0) {
		return error;
	}

	return CreateFatFileSystem(driverSlot, deviceIndex, selectedLunIndex, firstFileSystemSector, partition->sizeInK);
}


void putchar(char ch) __naked
{
    __asm
    push    ix
    ld      ix,#4
    add     ix,sp
    ld  a,(ix)
    call CHPUT
    pop ix
    ret
    __endasm;
}


void Locate(byte x, byte y)
{
	regs.Bytes.H = x + 1;
	regs.Bytes.L = y + 1;
	AsmCall(POSIT, &regs, REGS_MAIN, REGS_NONE);
}


#include "asmcall.c"