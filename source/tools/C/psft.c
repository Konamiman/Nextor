/* Partition size fix tool v1.0
   By Konamiman 5/2014

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196
        --no-std-crt0 crt0_msxdos_advanced.rel msxchar.rel
          asm.lib psft.c
   hex2bin -e com psft.ihx
   
   ASM.LIB, MSXCHAR.LIB and crt0msx_msxdos_advanced.rel
   are available at www.konamiman.com
   
   (You don't need MSXCHAR.LIB if you manage to put proper PUTCHAR.REL,
   GETCHAR.REL and PRINTF.REL in the standard Z80.LIB... I couldn't manage to
   do it, I get a "Library not created with SDCCLIB" error)
   
   Comments are welcome: konamiman@konamiman.com
*/

	
	/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

//These are available at www.konamiman.com
#include "asm.h"

	/* Typedefs */

typedef unsigned char bool;
typedef unsigned long ulong;

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

#define false (0)
#define true (!(false))
#define null ((void*)0)
	
#define _TERM0 0x00
#define _ALLOC 0x1B
#define _RDABS 0x2F
#define _WRABS 0x30
#define _DPARM 0x31
#define _TERM 0x62
#define _DOSVER 0x6F
#define _RDDRV 0x73
#define _WRDRV 0x74

#define MAX_FAT12_CLUSTER_COUNT 4084
#define MAX_FAT16_CLUSTER_COUNT 65524

#define Buffer ((void*)(0x8000))


	/* Strings */

const char* strTitle=
    "Partition Size Fix Tool v1.0\r\n"
    "By Konamiman, 5/2014\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: psft <drive>: [fix] \r\n"
    "\r\n"
	"This tool checks the cluster count calculated by DOS for a given volume\r\n"
	"and offers the possibility of fixing it if it is over the standard limits\r\n"
	"(4084 clusters for FAT12, 65524 clusters for FAT16)\r\n"
	"by slightly reducing the volume size in the boot sector.\r\n"
	"\r\n"
	"Run the tool as psft <drive>: first, and if it says that a fix is needed,\r\n"
	"run again adding the \"fix\" parameter to actually perform the fix.";
    
const char* strInvParam = "Invalid parameter";
const char* strCRLF = "\r\n";


	/* Global variables */

Z80_registers regs;
bool isNextor;
//char Buffer;
bool isFat16;
byte driveNumber;		//0=A:, etc


    /* Some handy code defines */

#define PrintNewLine() print(strCRLF)


    /* Function prototypes */

void Terminate(const char* errorMessage);
void print(char* s);


	/* MAIN */

int main(char** argv, int argc)
{
    //Buffer = ((byte*)0x8000);
	
	print(strTitle);

    if(argc == 0) {
        print(strUsage);
        Terminate(null);
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


void print(char* s) __naked
{
    __asm
    push    ix
    ld     ix,#4
    add ix,sp
    ld  l,(ix)
    ld  h,1(ix)
loop:
    ld  a,(hl)
    or  a
    jr  z,end
    ld  e,a
    ld  c,#2
    push    hl
    call    #5
    pop hl
    inc hl
    jr  loop
end:
    pop ix
    ret
    __endasm;    
}