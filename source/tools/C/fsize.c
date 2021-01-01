/* File size change tool v1.0
   By Konamiman 10/2014

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196
        --no-std-crt0 crt0_msxdos_advanced.rel fsize.c
   hex2bin -e com fsize.ihx
*/

/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "asmcall.h"
#include "types.h"
#include "dos.h"

/* Defines */

#define Buffer 0x9000

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
	"You can specify a new absolute <size>,\r\n"
	"or increase the current file size with +<size>.\r\n"
        "The new file space is filled randomly.\r\n"
	"\r\n"
	"Specify the size in Kilobytes by appending K,\r\n"
    "or in Megabytes by appending M.\r\n";
    
const char* strInvParam = "Invalid parameter";
const char* strCRLF = "\r\n";

/* Global variables */

byte ASMRUT[4];
byte OUT_FLAGS;
Z80_registers regs;
char* fileName;
bool isAbsoluteSize;
ulong newSize;

/* Some handy code defines */

#define PrintNewLine() print(strCRLF)

/* Function prototypes */

void Terminate(const char* errorMessage);
void TerminateWithDosError(byte errorCode);
void print(char* s);
void CheckDosVersion();
void ExtractParameters(char** argv, int argc);
bool IsDigit(char theChar);
bool FileExists(char* fileName);
byte OpenFile(char* fileName);
byte CreateFile(char* fileName);
ulong GetFileSize(byte fileHandle);
void CloseFile(byte fileHandle);
void SetFilePointer(byte fileHandle, ulong pointer);
void WriteOneByte(byte fileHandle, byte value);

	/* MAIN */
	
int main(char** argv, int argc)
{
	byte fileHandle;
	ulong oldSize;

    ASMRUT[0] = 0xC3;
	fileName = (char*)0x8000;

	print(strTitle);

    if(argc == 0) {
        print(strUsage);
        Terminate(null);
    }
	
	CheckDosVersion();
	ExtractParameters(argv, argc);
	
	if(FileExists(fileName))
		fileHandle = OpenFile(fileName);
	else
		fileHandle = CreateFile(fileName);
	
	oldSize = GetFileSize(fileHandle);
	
	if(!isAbsoluteSize)
		newSize += oldSize;
	
	if(newSize < oldSize) {
		CloseFile(fileHandle);
		Terminate("Can't reduce file size");
	}
	
	if(newSize > oldSize) {
		SetFilePointer(fileHandle, newSize-1);
		WriteOneByte(fileHandle, 0);
	}
	
	CloseFile(fileHandle);
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
	ulong multiplier;

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
	
	newSize = atol(sizePnt) * multiplier;
}


bool IsDigit(char theChar)
{
	return theChar >= '0' && theChar <= '9';
}


bool FileExists(char* fileName) 
{
	regs.Bytes.B = 0;
	regs.Words.DE = (int)fileName;
	regs.Words.IX = (int)Buffer;
	DosCall(_FFIRST, &regs, REGS_ALL, REGS_AF);
	return regs.Bytes.A == 0;
}


byte OpenFile(char* fileName) 
{
	regs.Bytes.A = 0;
	regs.Words.DE = (int)fileName;
	DosCall(_OPEN, &regs, REGS_MAIN, REGS_MAIN);

	if(regs.Bytes.A != 0) {
		TerminateWithDosError(regs.Bytes.A);
	}
	return regs.Bytes.B;
}


byte CreateFile(char* fileName)
{
	regs.Bytes.A = 0;
	regs.Bytes.B = 0x80;	//return error if file exists
	regs.Words.DE = (int)fileName;
	DosCall(_CREATE, &regs, REGS_MAIN, REGS_MAIN);

	if(regs.Bytes.A != 0) {
		TerminateWithDosError(regs.Bytes.A);
	}
	return regs.Bytes.B;
}


ulong GetFileSize(byte fileHandle)
{
	regs.Bytes.B = fileHandle;
	regs.Bytes.A = 2;	//Relative to end of file
	regs.Words.HL = 0;
	regs.Words.DE = 0;
	DosCall(_SEEK, &regs, REGS_MAIN, REGS_MAIN);

	if(regs.Bytes.A != 0) {
		TerminateWithDosError(regs.Bytes.A);
	}

	return (ulong)regs.Words.HL | (ulong)regs.Words.DE << 16;
}


void CloseFile(byte fileHandle)
{
	regs.Bytes.B = fileHandle;
	DosCall(_CLOSE, &regs, REGS_MAIN, REGS_NONE);
}


void SetFilePointer(byte fileHandle, ulong pointer)
{
	regs.Bytes.B = fileHandle;
	regs.Bytes.A = 0;	//Relative to beginning of file
	regs.Words.HL = (int)(pointer & 0xFFFF);
	regs.Words.DE = (int)(pointer >> 16);
	DosCall(_SEEK, &regs, REGS_MAIN, REGS_AF);

	if(regs.Bytes.A != 0) {
		TerminateWithDosError(regs.Bytes.A);
	}
}


void WriteOneByte(byte fileHandle, byte value)
{
	regs.Bytes.B = fileHandle;
	regs.Words.DE = (int)&value;
	regs.Words.HL = 1;
	DosCall(_WRITE, &regs, REGS_MAIN, REGS_AF);

	if(regs.Bytes.A != 0) {
		TerminateWithDosError(regs.Bytes.A);
	}
}

#define COM_FILE
#include "print_msxdos.c"
#include "printf.c"
#include "asmcall.c"
