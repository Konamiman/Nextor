/* Volume size fix tool v1.0
   By Konamiman 5/2014

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196
        --no-std-crt0 crt0_msxdos_advanced.rel
        vsft.c
   hex2bin -e com vsft.ihx
*/
	
	/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "strcmpi.h"
#include "asmcall.h"
#include "types.h"
#include "dos.h"

	/* Typedefs */

typedef struct {
	byte jumpInstruction[3];
	char oemNameString[8];
	uint sectorSize;
	byte sectorsPerCluster;
	uint reservedSectors;
	byte numberOfFats;
	uint rootDirectoryEntries;
	uint smallSectorCount;
	byte mediaId;
	uint sectorsPerFat;
	uint sectorsPerTrack;
	uint numberOfHeads;
	union {
		struct {
			ulong hiddenSectors;
			ulong bigSectorCount;
			byte physicalDriveNum;
			byte reserved;
			byte extendedBlockSignature;
			ulong serialNumber;
			char volumeLabelString[11];
			char fatTypeString[8];
		} standard;
		struct {
			uint hiddenSectors;
			byte z80JumpInstruction[2];
			char volIdString[6];
			byte dirtyDiskFlag;
			ulong serialNumber;
			char volumeLabelString[11];
			char fatTypeString[8];
			byte z80BootCode;
		} DOS220;
	} params;
} fatBootSector;


	/* Defines */

#define MAX_FAT12_CLUSTER_COUNT 4084
#define MAX_12BIT_CLUSTER_COUNT 4095
#define MAX_FAT16_CLUSTER_COUNT 65524


	/* Strings */

const char* strTitle=
    "Volume Size Fix Tool v1.0\r\n"
    "By Konamiman, 5/2014\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: vsft <drive>: [fix] \r\n"
    "\r\n"
	"This tool checks the cluster count calculated by DOS for a given volume\r\n"
	"and offers the possibility of fixing it if it is over the standard limits\r\n"
	"(4084 clusters for FAT12, 65524 clusters for FAT16)\r\n"
	"by appropriately reducing the volume size in the boot sector.\r\n"
	"\r\n"
	"Run the tool as 'vsft <drive>:' first, and if it says that a fix is needed,\r\n"
	"run again adding the \"fix\" parameter to actually perform the fix.\r\n"
	"\r\n"
	"Fixing FAT16 volumes is supported when running the tool in Nextor.\r\n"
	"DO NOT try to fix a FAT16 volume when running MSX-DOS with a FAT16 patch!\r\n";
    
const char* strInvParam = "Invalid parameter";
const char* strCRLF = "\r\n";


	/* Global variables */

byte ASMRUT[4];
byte OUT_FLAGS;
Z80_registers regs;
bool isNextor;
fatBootSector* Buffer = (fatBootSector*)0x8000;
bool isFat16;
int driveNumber;		//0=A:, etc
bool doFix;
uint totalClusters;
ulong totalSectors;
byte sectorsPerCluster;
int sectorsToDecreaseForFix;


    /* Some handy code defines */

#define PrintNewLine() printf(strCRLF)
#define StringIs(a, b) (strcmpi(a,b)==0)


    /* Function prototypes */

void Terminate(const char* errorMessage);
void TerminateWithDosError(byte errorCode);
void CheckDosVersion();
void ExtractParameters(char** argv, int argc);
void GetDriveInfo();
void PrintDriveInfo();
void PrintSizeInK(unsigned long size);
void CalculateSectorsToDecreaseForFix();
void DoFix();
void PrintFixInfo();
void ReadBootSector();
void FixVolumeSize();
void WritebootSector();
void print(char* s);


	/* MAIN */

int main(char** argv, int argc)
{
    ASMRUT[0] = 0xC3;
	print(strTitle);

    if(argc == 0) {
        print(strUsage);
        Terminate(null);
    }
	
	CheckDosVersion();
	ExtractParameters(argv, argc);
	GetDriveInfo();
	PrintDriveInfo();
	CalculateSectorsToDecreaseForFix();
	
	if(sectorsToDecreaseForFix == 0) {
		print("\r\nVolume size is correct. No need to fix it.\r\n");
		Terminate(null);
	} else if(doFix) {
		DoFix();
	} else {
		PrintFixInfo();
	}
	
	Terminate(null);
	return 0;
}


	/* Functions */
	
void Terminate(const char* errorMessage)
{
    if(errorMessage != NULL) {
        printf("\r\x1BK*** %s\r\n", errorMessage);
    }
    
    regs.Bytes.B = (errorMessage == NULL ? 0 : 1);
    DosCall(_TERM, &regs, REGS_MAIN, REGS_NONE);
}


void TerminateWithDosError(byte errorCode)
{
    regs.Bytes.B = errorCode;
    DosCall(_TERM, &regs, REGS_MAIN, REGS_NONE);
}


void CheckDosVersion()
{
	regs.Bytes.B = 0x5A;
	regs.Words.HL = 0x1234;
	regs.UWords.DE = 0xABCD;
	regs.Words.IX = 0;
    DosCall(_DOSVER, &regs, REGS_ALL, REGS_ALL);
	
    if(regs.Bytes.B < 2) {
        Terminate("This program is for MSX-DOS 2 only.");
    }
	
	isNextor = (regs.Bytes.IXh == 1);
}


void ExtractParameters(char** argv, int argc)
{
	if(argc > 2) {
        Terminate(strInvParam);
	}
	
	if(strlen(argv[0]) > 2 || argv[0][1] != ':') {
		Terminate(strInvParam);
	}
	
	driveNumber = (argv[0][0] | 32) - 'a';
	if(driveNumber < 0 || driveNumber > ('h'-'a')) {
		Terminate(strInvParam);
	}
	
	if(argc == 2) {
		if(StringIs(argv[1], "fix")) {
			doFix = true;
		} else {
			Terminate(strInvParam);
		}
	} else {
		doFix = false;
	}
}

void GetDriveInfo()
{
	regs.Words.DE = (int)Buffer;
	regs.Bytes.L = driveNumber + 1;
	DosCall(_DPARM, &regs, REGS_MAIN, REGS_MAIN);
	if(regs.Bytes.A != 0) {
		TerminateWithDosError(regs.Bytes.A);
	}
	
	if(isNextor) {
		totalSectors = *((ulong*)((byte*)Buffer+24));
		isFat16 = (*((byte*)Buffer+28) == 1) ? true : false;
	} else {
		totalSectors = *((uint*)((byte*)Buffer+9));
		isFat16 = false;
	}

	if(totalSectors == 0) {
		Terminate("Reported volume size is zero (are you trying to access a FAT16 volume from MSX-DOS?)");
	}
	
	regs.Bytes.E = driveNumber + 1;
	DosCall(_ALLOC, &regs, REGS_MAIN, REGS_MAIN);
	totalClusters = regs.UWords.DE;
	sectorsPerCluster = regs.Bytes.A;
}

void PrintDriveInfo()
{
	printf("Drive: %c:\r\n", driveNumber+'A');
	printf("Size:  "); PrintSizeInK(totalSectors/2); PrintNewLine();
	printf("Cluster count: %u\r\n", totalClusters);
	if(sectorsPerCluster == 1) {
		print("Cluster size:  512 bytes\r\n");
	} else {
		printf("Cluster size:  %i KBytes\r\n", sectorsPerCluster / 2);
	}
	
	if(isNextor) {
		printf("Filesystem:    %s\r\n", isFat16 ? "FAT16" : "FAT12");
	} else if(totalClusters > MAX_12BIT_CLUSTER_COUNT) {
		Terminate("Cluster count does not fit in 12 bits - this is not supposed to be supported by MSX-DOS!");
	} else {
		print("Filesystem:    FAT12 assumed (MSX-DOS does not support anything else)\r\n");
	}
}

void PrintSizeInK(unsigned long size)
{
    int remaining;

    if(size < 1024) {
        printf("%i KBytes", size);
        return;
    }

    remaining = size & 1023;
    if(remaining > 1000) {
        remaining = 999;
    }
    size >>= 10;
    if(size < 1024) {
        printf("%i.%i MBytes", (int)size, remaining/100);
        return;
    }
    
    remaining = size & 1023;
    if(remaining > 1000) {
        remaining = 999;
    }
    size >>= 10;
    printf("%i.%i GBytes", (int)size, remaining/100);
}


void CalculateSectorsToDecreaseForFix()
{
	if(isFat16 && (totalClusters > MAX_FAT16_CLUSTER_COUNT)) {
		sectorsToDecreaseForFix = (totalClusters - MAX_FAT16_CLUSTER_COUNT) * sectorsPerCluster;
	} else if(!isFat16 && (totalClusters > MAX_FAT12_CLUSTER_COUNT)) {
		sectorsToDecreaseForFix = (totalClusters - MAX_FAT12_CLUSTER_COUNT) * sectorsPerCluster;
	} else {
		sectorsToDecreaseForFix = 0;
	}
}


void DoFix()
{
	ReadBootSector();
	FixVolumeSize();
	WritebootSector();
	print("Fix applied!\r\n");
}


void PrintFixInfo()
{
	printf("\r\nCluster count exceeds the maximum allowed (%u).\r\n", 
		(isFat16 ? MAX_FAT16_CLUSTER_COUNT : MAX_FAT12_CLUSTER_COUNT));
	printf("This can be fixed by reducing the volume size by %i KBytes.\r\n",
		sectorsToDecreaseForFix/2);
	print("Run the tool again adding the 'fix' parameter to apply the fix.\r\n");
}


void ReadBootSector()
{
	print("\r\nReading boot sector...\r\n");

	regs.Words.DE = (int)Buffer;
	DosCall(_SETDTA, &regs, REGS_MAIN, REGS_MAIN);

	if(isNextor) {
		regs.Bytes.A = driveNumber;
		regs.Bytes.B = 1;
		regs.Words.HL = 0;
		regs.Words.DE = 0;
		DosCall(_RDDRV, &regs, REGS_MAIN, REGS_MAIN);
	} else {
		regs.Bytes.L = driveNumber;
		regs.Bytes.H = 1;
		regs.Words.DE = 0;
		DosCall(_RDABS, &regs, REGS_MAIN, REGS_MAIN);
	}

	if(regs.Bytes.A != 0) {
		TerminateWithDosError(regs.Bytes.A);
	}
}


void FixVolumeSize()
{
	ulong totalSectors;

	totalSectors = Buffer->smallSectorCount;
	if(totalSectors == 0) {
		totalSectors = Buffer->params.standard.bigSectorCount;
	}

	totalSectors -= sectorsToDecreaseForFix;

	if(totalSectors <= 0xFFFF) {
		Buffer->smallSectorCount = (uint)totalSectors;
	} else {
		Buffer->smallSectorCount = 0;
		Buffer->params.standard.bigSectorCount = totalSectors;
	}
}


void WritebootSector()
{
	print("Writing updated boot sector...\r\n");

	regs.Words.DE = (int)Buffer;
	DosCall(_SETDTA, &regs, REGS_MAIN, REGS_MAIN);

	if(isNextor) {
		regs.Bytes.A = driveNumber;
		regs.Bytes.B = 1;
		regs.Words.HL = 0;
		regs.Words.DE = 0;
		DosCall(_WRDRV, &regs, REGS_MAIN, REGS_MAIN);
	} else {
		regs.Bytes.L = driveNumber;
		regs.Bytes.H = 1;
		regs.Words.DE = 0;
		DosCall(_WRABS, &regs, REGS_MAIN, REGS_MAIN);
	}

	if(regs.Bytes.A != 0) {
		TerminateWithDosError(regs.Bytes.A);
	}
}


#define COM_FILE
#include "printf.c"
#include "asmcall.c"
#include "strcmpi.c"
