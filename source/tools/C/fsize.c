/* File size change tool v1.0
   By Konamiman 10/2014

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196
        --no-std-crt0 crt0_msxdos_advanced.rel msxchar.rel
          asm.lib fsize.c
   hex2bin -e com fsize.ihx
   
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

#define _TERM0 0
#define _TERM 0x62
#define _DOSVER 0x6F

/* Strings */

const char* strTitle=
    "File Size Change Tool v1.0\r\n"
    "By Konamiman, 10/2014\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: fsize <file> [+]<size>[K|M]\r\n"
    "\r\n"
	"This tool creates a file with the specified size,\r\n"
	"or modifies the size of an existing file.\r\n"
	"\r\n"
	"You can specify an absolute <size>,\r\n"
	"or increase the current file size with +<size>\r\n"
	"\r\n"
	"Specify the size in Kilobytes by appending K,\r\n"
    "or in Megabytes by appending M.\r\n";
    
const char* strInvParam = "Invalid parameter";
const char* strCRLF = "\r\n";

/* Global variables */

Z80_registers regs;
char* fileName;
bool isAbsoluteSize;
ulong newSize;
ulong multiplier;

/* Some handy code defines */

#define PrintNewLine() print(strCRLF)

/* Function prototypes */

void Terminate(const char* errorMessage);
void TerminateWithDosError(byte errorCode);
void print(char* s);
void CheckDosVersion();
void ExtractParameters(char** argv, int argc);
bool IsDigit(char theChar);


	/* MAIN */
	
int main(char** argv, int argc)
{
	fileName = (char*)0x8000;

	print(strTitle);

    if(argc == 0) {
        print(strUsage);
        Terminate(null);
    }
	
	CheckDosVersion();
	ExtractParameters(argv, argc);

	
	
	
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


void ExtractParameters(char** argv, int argc)
{
	char* sizePnt;
	char lastSizeChar;
	char firstSizeChar;

	if(argc != 2) {
        Terminate(strInvParam);
	}
	
	strcpy(fileName, argv[0]);
	sizePnt = argv[1];
	firstSizeChar = sizePnt[0];
	if(firstSizeChar == '+') {
		isAbsoluteSize = false;
		sizePnt++;
	} else  if(!IsDigit(firstSizeChar)){
		Terminate(strInvParam);
	} else {
		isAbsoluteSize = true;
	}

	multiplier = 1;	
	lastSizeChar = sizePnt[strlen(sizePnt)-1] | 32;
	if(lastSizeChar == 'k') {
		multiplier = 1024;
		sizePnt[strlen(sizePnt)-1] = 0;
	} else if(lastSizeChar == 'm') {
		multiplier = (ulong)1024 * (ulong)1024;
		sizePnt[strlen(sizePnt)-1] = 0;
	} else if(!IsDigit(lastSizeChar)) {
		Terminate(strInvParam);
	}
	
	newSize = atol(sizePnt);
}


bool IsDigit(char theChar)
{
	return theChar >= '0' && theChar <= '9';
}