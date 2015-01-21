/* DSK emulation configuration file creation tool v1.0
   By Konamiman 2/2015

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196
          --no-std-crt0 crt0_msxdos_advanced.rel msxchar.rel
          asm.lib emufile.c
   hex2bin -e com emufile.ihx
   
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

	/* Defines */

#define false (0)
#define true (!(false))
#define null ((void*)0)

#define MallocBase 0xA000

#define _TERM0 0
#define _FFIRST 0x40
#define _OPEN 0x43
#define _CREATE 0x44
#define _CLOSE 0x45
#define _WRITE 0x49
#define _SEEK 0x4A
#define _TERM 0x62
#define _DOSVER 0x6F

/* Strings */

const char* strTitle=
    "DSK Emulation Configuration File Creation Tool v1.0\r\n"
    "By Konamiman, 2/2015\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: emufile [<options>] <files> [<files> ...]\r\n"
    "\r\n"
    "<files>: Disk image files to emulate. Can contain wildcards.\r\n"
    "\r\n"
    "<options>:\r\n"
    "\r\n"
    "- o <file>: Path and name of the generated file.\r\n"
    "            Default is \"\\NEXT_DSK.DAT\"\r\n"
    "            (generated in the root directory of current drive).\r\n"
    "            If <file> ends with \"\\\", \"NEXT_DSK.DAT\" is appended.\r\n"
    "\r\n"
    "- b <number>: The index of the image to mount at boot time.\r\n"
    "              Must be 1-9 or A-Z. Default is 1.\r\n"
    "\r\n"
    "- a <address>: Page 3 address for a 16 byte work area.\r\n"
    "               Must be a hexadecimal number.\r\n"
    "               If missing or 0, the area is allocated at boot time.\r\n"
    "\r\n"
    "- r : Reset the computer after generating the file.\r\n";

const char* strInvParam = "Invalid parameter";
const char* strCRLF = "\r\n";

/* Global variables */

Z80_registers regs;
void* mallocPointer;

/* Some handy code defines */

#define PrintNewLine() print(strCRLF)

/* Function prototypes */

void Initialize();
void ProcessArguments(char** argv, int argc);
int ProcessOption(char optionLetter, char* optionValue);
void ProcessFilename(char* fileName);
void GenerateFile();
void ProcessOutputFileOption(char* optionValue);
void ProcessBootIndexOption(char* optionValue);
void ProcessWorkAreaAddressOption(char* optionValue);
void ProcessResetOption();

void Terminate(const char* errorMessage);
void TerminateWithDosError(byte errorCode);
void print(char* s);
void CheckDosVersion();
void* malloc(int size);

	/* MAIN */
	
int main(char** argv, int argc)
{
	print(strTitle);
	
	Initialize();
	ProcessArguments(argv, argc);
	GenerateFile();
	
	printf("%i\r\n", malloc(0));
	printf("%i\r\n", malloc(100));
	Terminate(null);
	return 0;
}

/* Functions */

void Initialize()
{
	mallocPointer = (void*)MallocBase;
}

void ProcessArguments(char** argv, int argc) 
{
	int i;
	char* currentArg;

	if(argc == 0) {
        print(strUsage);
        Terminate(null);
    }
	
    for(i=0; i<argc; i++) {
	    currentArg = argv[i];
		if(currentArg[0] == '-') {
		    i += ProcessOption(currentArg[1], argv[i+1]);
		} else {
		    ProcessFilename(currentArg);
		}
	}
}

int ProcessOption(char optionLetter, char* optionValue)
{
    optionLetter |= 32;
	
	if(optionLetter == 'o') {
		ProcessOutputFileOption(optionValue);
		return 1;
	} 
	
	if(optionLetter == 'b') {
	    ProcessBootIndexOption(optionValue);
		return 1;
	}
	
	if(optionLetter == 'a') {
	    ProcessWorkAreaAddressOption(optionValue);
		return 1;
	}
	
	if(optionLetter == 'r') {
	    ProcessResetOption();
		return 0;
	}

	Terminate(strInvParam);
	return 0;
}

void ProcessOutputFileOption(char* optionValue)
{
	printf("o: %s\r\n", optionValue);
}

void ProcessBootIndexOption(char* optionValue)
{
	printf("b: %s\r\n", optionValue);
}

void ProcessWorkAreaAddressOption(char* optionValue)
{
	printf("a: %s\r\n", optionValue);
}

void ProcessResetOption()
{
	printf("r\r\n");
}

void ProcessFilename(char* fileName) 
{ 
	printf("f: %s\r\n", fileName);
}

void GenerateFile() { }

void Terminate(const char* errorMessage)
{
    if(errorMessage != NULL) {
        printf("\r\x1BK*** %s\r\n", errorMessage);
    }
    
    regs.Bytes.B = (errorMessage == NULL ? 0 : 1);
    DosCall(_TERM, &regs, REGS_MAIN, REGS_NONE);
    DosCall(_TERM0, &regs, REGS_MAIN, REGS_NONE);
}


void TerminateWithDosError(byte errorCode)
{
    regs.Bytes.B = errorCode;
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


void CheckDosVersion()
{
    DosCall(_DOSVER, &regs, REGS_ALL, REGS_ALL);
	
    if(regs.Bytes.B < 2) {
        Terminate("This program is for MSX-DOS 2 only.");
    }
}

void* malloc(int size)
{
	void* value = mallocPointer;
	mallocPointer += size;
	return value;
}

