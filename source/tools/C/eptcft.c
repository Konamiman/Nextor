/* Extended partition type code fix tool v1.0
   By Konamiman 11/2023

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196
        --no-std-crt0 crt0_msxdos_advanced.rel
        eptcft.c
   hex2bin -e com eptcft.ihx
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
#include "partit.h"

	/* Defines */

#define PTYPE_EXTENDED_CHS 5
#define PTYPE_EXTENDED_LBA 15

	/* Strings */

const char* strTitle=
    "Extended Partition Type Code Fix Tool v1.0\r\n"
    "By Konamiman, 11/2023\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: eptcft <drive>: [fix|unfix] \r\n"
	"       eptcft <device>[-<LUN>] <slot> [fix|unfix] \r\n"
    "\r\n"
	"This tool checks the partition type code used by extended partitions\r\n"
	"existing on a given device controlled by a Nextor driver.\r\n"
	"If <drive> is specified, the device the drive is mapped to is used.\r\n"
	"If <slot> is 0, the primary Nextor controller is used.\r\n"
	"\r\n"
	"Since Nextor 2.1.2 FDISK creates extended partitions with code 15\r\n"
	"(extended LBA), in older Nextor versions FDISK uses code 5 (extended CHS)\r\n"
	"for new extended partitions, which is incorrect.\r\n"
	"\r\n"
	"Adding \"fix\" will change the extended partitions that have code 5 to 15\r\n"
	"in the device, adding \"unfix\" will do the opposite (might be needed for\r\n"
	"old partition handling tools that assume code 5 for extended partitions).\r\n"
	"Run without neither \"fix\" nor \"fix\" to see partition information.\r\n";
    
const char* strInvParam = "Invalid parameter";
const char* strCRLF = "\r\n";


	/* Global variables */

byte ASMRUT[4];
byte OUT_FLAGS;
Z80_registers regs;
Z80_registers regs2;
bool isNextor;
masterBootRecord* mbrBuffer = (masterBootRecord*)0x8000;
driveLetterInfo* dliBuffer = (driveLetterInfo*)0x8200;
driverInfo* driverInfoBuffer = (driverInfo*)0x8200;
char* stringBuffer = (char*)0x8000;
int driveNumber;		//-1=No drive specified, 0=A:, etc
byte deviceNumber;
byte lunNumber;
byte slotNumber;
bool doFix;
bool doUnfix;
int chsPartitionsCount = 0;
int lbaPartitionsCount = 0;
void ReadOrWriteSector(bool write);
#define SECTOR_NUMBER_BUFFER 0x8200


    /* Some handy code defines */

#define PrintNewLine() printf(strCRLF)
#define StringIs(a, b) (strcmpi(a,b)==0)


    /* Function prototypes */

void Terminate(const char* errorMessage);
void TerminateWithDosError(byte errorCode);
void CheckDosVersion();
void ExtractParameters(char** argv, int argc);
void ProcessParameters();
void DoDosCall(byte functionCode);
void ScanAndFixPartitions();

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
	ProcessParameters();
	ScanAndFixPartitions();
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
	
    if(regs.Bytes.B < 2 || regs.Bytes.IXh != 1) {
        Terminate("This program is for Nextor only.");
    }
}


void ExtractParameters(char** argv, int argc)
{
	int fixParamIndex;

	if(argv[0][1] == ':') {
		driveNumber = (argv[0][0] | 32) - 'a';
		fixParamIndex = 1;
	}
	else {
		if(argc < 2) {
			Terminate(strInvParam);
		}
		driveNumber = -1;

		deviceNumber = argv[0][0];
		if(deviceNumber < '1' || deviceNumber > '7') {
			Terminate(strInvParam);
		}
		deviceNumber -= '0';

		if(argv[0][1] == '-') {
			lunNumber = argv[0][2];
			if(lunNumber < '0' || lunNumber > '7') {
				Terminate(strInvParam);
			}
			lunNumber -= '0'; - '0';
		}
		else {
			lunNumber = 1;
		}

		slotNumber = argv[1][0];
		if(slotNumber < '0' || slotNumber > '3') {
			Terminate(strInvParam);
		}
		slotNumber -= '0';

		if(argv[1][1] == '-') {
			if(argv[1][2] < '0' || argv[1][2] > '3') {
				Terminate(strInvParam);
			}
			slotNumber += 0x80 + ((argv[1][2] - '0') << 2);
		}

		fixParamIndex = 2;
	}

	if(argc < fixParamIndex + 1) {
		return;
	}

	if(strcmpi(argv[fixParamIndex], "fix") == 0) {
		doFix = true;
	}
	else if(strcmpi(argv[fixParamIndex], "unfix") == 0) {
		doUnfix = true;
	}
	else {
		Terminate(strInvParam);
	}
}


void ProcessParameters() {
	int i;

	if(driveNumber != -1) {
		regs.Bytes.A = driveNumber;
		regs.Words.HL = (int)dliBuffer;
		DoDosCall(_GDLI);

		switch(dliBuffer->driveStatus) {
			case 0:
				Terminate("The specified drive is unassigned");
				break;
			case 1:
				break;
			case 3:
				Terminate("The specified drive is mapped to a mounted file");
				break;
			case 4:
				Terminate("The specified drive is mapped to the RAM disk");
				break;
			default:
				printf("The specified drive has unknown status: %i\r\n", dliBuffer->driveStatus);
				TerminateWithDosError(1);
				break;
		}

		slotNumber = dliBuffer->driverSlotNumber;
		deviceNumber = dliBuffer->deviceIndex;
		lunNumber = dliBuffer->logicalUnitNumber;

		if(deviceNumber == 0) {
			Terminate("The specified drive is mapped to a MSX-DOS controller");
		}
	}
	else if(slotNumber == 0) {
		regs.Bytes.A = 1;  //Primary Nextor controller
		regs.Words.HL = (int)driverInfoBuffer;
		DoDosCall(_GDRVR);
		slotNumber = driverInfoBuffer->slot;
	}

	regs.Bytes.A = 0;
	regs.Bytes.D = slotNumber;
	regs.Bytes.E = 0xFF;
	regs.Words.HL = (int)driverInfoBuffer;
	DoDosCall(_GDRVR);
	for(int i=31; i>0; i--) {
		if(driverInfoBuffer->driverName[i] != ' ') {
			driverInfoBuffer->driverName[i+1] = '\0';
			break;
		}
	}
	printf("Driver: %s v%i.%i.%i in slot ", driverInfoBuffer->driverName, driverInfoBuffer->versionMain, driverInfoBuffer->versionSec, driverInfoBuffer->versionRev);
	if(slotNumber & 0x80) {
		printf("%i-%i\r\n", slotNumber & 3, (slotNumber >> 2) & 3);
	}
	else {
		printf("%i\r\n", slotNumber);
	}

	regs.Bytes.A = slotNumber;
	regs.Bytes.B = 0xFF;
	regs.Words.DE = DEV_INFO;
	regs.Words.HL = (int)&regs2;
	regs2.Bytes.A = deviceNumber;
	regs2.Bytes.B = 2;	//Device name string
	regs2.Words.HL = (int)stringBuffer;
	DoDosCall(_CDRVR);
	if(regs2.Bytes.IXh == 0) {
		for(int i=31; i>0; i--) {
			if(stringBuffer[i] != ' ') {
				stringBuffer[i+1] = '\0';
				break;
			}
		}
		printf("Device %i, %s\r\n", deviceNumber, stringBuffer);
	}
	else {
		printf("Device %i\r\n", deviceNumber);
	}
}

void DoDosCall(byte functionCode)
{
    DosCall(functionCode, &regs, REGS_ALL, REGS_ALL);
    if(regs.Bytes.A != 0 && (functionCode != _GPART || regs.Bytes.A != _IPART)) {
        TerminateWithDosError(regs.Bytes.A);
    }
}

void ScanAndFixPartitions()
{
	byte mainPartition;
	byte extendedPartition;
	byte partitionType;

	if(doFix) {
		print("\r\nScanning and fixing partitions...\r\n\r\n");
	}
	else if(doUnfix) {
		print("\r\nScanning and unfixing partitions...\r\n\r\n");
	}
	else {
		print("\r\nScanning partitions...\r\n\r\n");
	}

	extendedPartition = 0;
	while(true) {
		regs.Bytes.A = slotNumber;
		regs.Bytes.B = 0xFF;
		regs.Bytes.D = deviceNumber;
		regs.Bytes.E = lunNumber;
		regs.Bytes.H = 0x82;  //Main partition 2 plus the "get partition entry sector number" flag
		regs.Bytes.L = extendedPartition;
		DoDosCall(_GPART);

		if(regs.Bytes.A != 0) {
			break;
		}

		*((int*)(SECTOR_NUMBER_BUFFER)) = regs.Words.DE;
		*((int*)(SECTOR_NUMBER_BUFFER+2)) = regs.Words.HL;

		ReadOrWriteSector(false);

		partitionType = mbrBuffer->primaryPartitions[1].partitionType;
		if(partitionType == PTYPE_EXTENDED_CHS) {
			chsPartitionsCount++;
			if(doFix) {
				partitionType = mbrBuffer->primaryPartitions[1].partitionType = PTYPE_EXTENDED_LBA;
				ReadOrWriteSector(true);
			}
		}
		else if(partitionType == PTYPE_EXTENDED_LBA) {
			lbaPartitionsCount++;
			if(doUnfix) {
				partitionType = mbrBuffer->primaryPartitions[1].partitionType = PTYPE_EXTENDED_CHS;
				ReadOrWriteSector(true);
			}
		}
		else {
			break;
		}

		extendedPartition++;
	}

	if(chsPartitionsCount == 0 && lbaPartitionsCount == 0) {
		print("This device has no extended partitions.\r\n");
		return;
	}

	if(doFix) {
		if(chsPartitionsCount > 0) {
			printf("%i partitions fixed\r\n(partition code changed from \"extended CHS\" to \"extended LBA\")\r\n", chsPartitionsCount);
		}
		else {
			print("No partitions with partition code \"extended CHS\" found, nothing to fix.\r\n");
		}	
	}
	else if(doUnfix) {
		if(lbaPartitionsCount > 0) {
			printf("%i partitions unfixed\r\n(partition code changed from \"extended LBA\" to \"extended CHS\")\r\n", lbaPartitionsCount);
		}
		else {
			print("No partitions with partition code \"extended LBA\" found, nothing to fix.\r\n");
		}
	}
	else {
		if(chsPartitionsCount > 0) {
			printf("%i partitions found with code 5 (extended CHS)\r\n", chsPartitionsCount);
		}
		if(lbaPartitionsCount > 0) {
			printf("%i partitions found with code 15 (extended LBA)\r\n", lbaPartitionsCount);
		}
		print("No changes made to any partition.\r\n");
	}
}

void ReadOrWriteSector(bool write) {
	regs.Bytes.A = slotNumber;
	regs.Bytes.B = 0xFF;
	regs.Words.DE = DEV_RW;
	regs.Words.HL = (int)&regs2;
	regs2.Bytes.A = deviceNumber;
	regs2.Bytes.C = lunNumber;
	regs2.Flags.C = write ? 1 : 0;
	regs2.Bytes.B = 1;  //Read/write 1 sector
	regs2.Words.HL = (int)mbrBuffer;
	regs2.Words.DE = (int)SECTOR_NUMBER_BUFFER;
	DoDosCall(_CDRVR);
}

#define COM_FILE
#include "print_msxdos.c"
#include "printf.c"
#include "asmcall.c"
#include "strcmpi.c"
