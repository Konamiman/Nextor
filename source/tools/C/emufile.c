/* DSK emulation configuration file creation tool for Nextor v1.0
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

typedef struct {
    byte alwaysFF;
    char filename[13];
    byte attributes;
    byte timeOfModification[2];
    byte dateOfModification[2];
    unsigned int startCluster;
    unsigned long fileSize;
    byte logicalDrive;
    byte internal[38];
} FileInfoBlock;

typedef struct {
    char signature[16];
    byte numberOfEntriesInImagesTable;
    byte indexOfImageToMountAtBoot;
    uint workAreaAddress;
    byte reserved[4];
} GeneratedFileHeader;

typedef struct {
    byte deviceIndex;
    byte logicalUnitNumber;
    ulong firstFileSector;
    uint fileSizeInSector;
} GeneratedFileTableEntry;
    
	/* Defines */

#define false (0)
#define true (!(false))
#define null ((void*)0)

#define IS_NEXTOR (1 << 7)
#define IS_DEVICE_BASED (1)

#define MallocBase 0x8000

#define MaxFilesToProcess 32

#define CALSLT 0x001C
#define EXPTBL 0xFCC1

#define _TERM0 0
#define _DPARM 0x31
#define _FFIRST 0x40
#define _FNEXT 0x41
#define _OPEN 0x43
#define _CREATE 0x44
#define _CLOSE 0x45
#define _WRITE 0x49
#define _TERM 0x62
#define _DOSVER 0x6F
#define _GDRVR 0x78
#define _GDLI 0x79

#define _NOFIL 0xD7

/* Strings */

const char* strTitle=
    "DSK Emulation Configuration File Creation Tool for Nextor v1.0\r\n"
    "By Konamiman, 2/2015\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: emufile [<options>] <files> [<files> ...]\r\n"
    "\r\n"
    "<files>: Disk image files to emulate. Can contain wildcards.\r\n"
    "\r\n"
    "<options>:\r\n"
    "\r\n"
    "- o <file>: Path and name of the generated configuration file.\r\n"
    "            Default is \"\\NEXT_DSK.DAT\"\r\n"
    "            (generated in the root directory of current drive).\r\n"
    "            If <file> ends with \"\\\" or \":\", \"NEXT_DSK.DAT\" is appended.\r\n"
    "- b <number>: The index of the image file to mount at boot time.\r\n"
    "              Must be 1-9 or A-W. Default is 1.\r\n"
    "- a <address>: Page 3 address for the 16 byte work area.\r\n"
    "               Must be a hexadecimal number between C000 and FFEF.\r\n"
    "               If missing or 0, the work area is allocated at boot time.\r\n"
    "- r : Reset the computer after generating the file.\r\n"
    "- p : Print filenames and associated keys.\r\n";

const char* strInvParam = "Invalid parameter";
const char* strCRLF = "\r\n";

/* Global variables */

Z80_registers regs;
void* mallocPointer;
char* outputFileName;
int bootFileIndex;
bool autoReset;
bool printFilenames;
uint workAreaAddress;
FileInfoBlock* fib;
int totalFilesProcessed;
byte* driveInfo;
byte* driveParameters;
byte* fileContentsBase;
byte* fileNamesBase;
byte* fileNamesAppendAddress;

/* Some handy code defines */

#define PrintNewLine() print(strCRLF)
#define InvalidParameter() Terminate(strInvParam)
#define PrimaryControllerSlot() (*(byte*)0xF348)

/* Function prototypes */

void CheckPreconditions();
void CheckPrimaryControllerIsNextor();
void Initialize();
void ProcessArguments(char** argv, int argc);
int ProcessOption(char optionLetter, char* optionValue);
void ProcessFilename(char* fileName);
void TooManyFiles();
void StartSearchingFiles(char* fileName);
void ProcessFileFound();
void GetDriveInfoForFileInFib();
void CheckControllerForFileInFib();
ulong GetFirstDriveSectorForFileInFib();
ulong GetFirstFileSectorForFileInFib();
void AddFileInFibToFilesTable(ulong sector);
void AddFileInFibToFilenamesInfo();
void GenerateFile();
void ProcessOutputFileOption(char* optionValue);
void ProcessBootIndexOption(char* optionValue);
void ProcessWorkAreaAddressOption(char* optionValue);
void ProcessResetOption();
void ProcessPrintFilenamesOption();

void Terminate(const char* errorMessage);
void TerminateWithDosError(byte errorCode);
void print(char* s);
void CheckDosVersion();
void* malloc(int size);
uint ParseHex(char* hexString);
void DoDosCall(byte functionCode);
void ResetComputer();

	/* MAIN */
	
int main(char** argv, int argc)
{
	print(strTitle);
	
    CheckPreconditions();
	Initialize();
	ProcessArguments(argv, argc);
    if(totalFilesProcessed > 0) {
        GenerateFile();
        printf(
            "%s%s successfully generated!\r\n%i disk image file(s) registered\r\n",
            printFilenames ? "\r\n" : "", outputFileName, totalFilesProcessed);
        if(autoReset) {
            print("Resetting computer...");
            ResetComputer();
        }
    } else {
        print(strUsage);
    }
    
	Terminate(null);
	return 0;
}

/* Functions */

void CheckPreconditions()
{
    CheckDosVersion();
    CheckPrimaryControllerIsNextor();
}

void CheckPrimaryControllerIsNextor()
{
    byte flags;
    
    regs.Bytes.A = 0;
    regs.Bytes.D = PrimaryControllerSlot();
    regs.Bytes.E = 0xFF;
    regs.Words.HL = (int)MallocBase;
    
    DoDosCall(_GDRVR);
    
    flags = ((byte*)MallocBase)[4];
    if((flags & (IS_NEXTOR | IS_DEVICE_BASED)) != (IS_NEXTOR | IS_DEVICE_BASED)) {
        Terminate("The primary controller is not a Nextor kernel with a device-based driver.");
    }
}

void Initialize()
{
	mallocPointer = (void*)MallocBase;
	
	outputFileName = malloc(128);
	strcpy(outputFileName, "\\NEXT_DSK.DAT");
    
    fib = malloc(sizeof(FileInfoBlock));
    driveInfo = malloc(64);
    driveParameters = malloc(32);
	fileContentsBase = malloc(
        sizeof(GeneratedFileHeader) +
        (sizeof(GeneratedFileTableEntry) * MaxFilesToProcess));
    fileNamesBase = malloc(19 * MaxFilesToProcess);
    fileNamesAppendAddress = fileNamesBase;
    
	bootFileIndex = 1;
    autoReset = false;
    printFilenames = false;
    workAreaAddress = 0;
    totalFilesProcessed = 0;
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
		} else if(totalFilesProcessed < MaxFilesToProcess) {
		    ProcessFilename(currentArg);
		} else {
            TooManyFiles();
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
    
    if(optionLetter == 'p') {
	    ProcessPrintFilenamesOption();
		return 0;
	}

	InvalidParameter();
	return 0;
}

void ProcessOutputFileOption(char* optionValue)
{
	char* lastCharPointer;
	
	strcpy(outputFileName, optionValue);
	lastCharPointer = outputFileName + strlen(outputFileName) - 1;
	if(*lastCharPointer == '\\' || *lastCharPointer == ':') {
	    strcpy(lastCharPointer + 1, "NEXT_DSK.DAT");
	}
}

void ProcessBootIndexOption(char* optionValue)
{
    char index;
    
	if(optionValue[1] != 0) {
	    InvalidParameter();
	}
	
	index = *optionValue | 32;
	
	if(index >= '1' && index <= '9') {
        bootFileIndex = index - '0';
	} else if(index >= 'a' && index <= 'w') {
        bootFileIndex = index - 'a' + 10;
    } else {
        InvalidParameter();
    }
}

void ProcessWorkAreaAddressOption(char* optionValue)
{
	workAreaAddress = ParseHex(optionValue);
    
    if(workAreaAddress != 0 && (workAreaAddress < 0xC000 || workAreaAddress > 0xFFEF)) {
        InvalidParameter();
    }
}

void ProcessResetOption()
{
	autoReset = true;
}

void ProcessPrintFilenamesOption()
{
    printFilenames = true;
}

void ProcessFilename(char* fileName) 
{ 
    StartSearchingFiles(fileName);
    
    while(regs.Bytes.A == 0 && totalFilesProcessed < MaxFilesToProcess) {
        ProcessFileFound();
        DoDosCall(_FNEXT);
    } 
    
    if(regs.Bytes.A == 0 && totalFilesProcessed == MaxFilesToProcess) {
        TooManyFiles();
    }
 }

void TooManyFiles()
{
    printf("*** Too many files specified, maximum is %i\r\n", MaxFilesToProcess);
    Terminate(null);
}

void StartSearchingFiles(char* fileName)
{
    regs.Words.DE = (int)fileName;
    regs.Bytes.B = 0;
    regs.Words.IX = (int)fib;
    
    DoDosCall(_FFIRST);
}

void ProcessFileFound()
{
    char key;
	ulong sector;

	GetDriveInfoForFileInFib();
    CheckControllerForFileInFib();
    
    if(fib->fileSize < 512) {
        printf("*** %s is too small (< 512 bytes) or empty - skipped\r\n", fib->filename);
        return;
    }
    
    if(fib->fileSize >= (32768 * 1024)) {
        printf("*** %s is too big (> 32 MBytes) - skipped\r\n", fib->filename);
        return;
    }
    
	sector = GetFirstDriveSectorForFileInFib();
	sector += GetFirstFileSectorForFileInFib();
	AddFileInFibToFilesTable(sector);
	AddFileInFibToFilenamesInfo();
	
    totalFilesProcessed++;

    if(printFilenames) {
        key = totalFilesProcessed < 10 ? totalFilesProcessed + '0' : totalFilesProcessed - 10 + 'A';
        printf("%c -> %s\r\n", key, fib->filename);
    }
}

void GetDriveInfoForFileInFib()
{
	regs.Bytes.A = fib->logicalDrive - 1;
    regs.Words.HL = (int)driveInfo;
    DoDosCall(_GDLI);
}

void CheckControllerForFileInFib()
{
    if(driveInfo[0] != 1 || driveInfo[1] != PrimaryControllerSlot()) {
        printf("*** Drive %c: is not controlled by the primary Nextor kernel\r\n", fib->logicalDrive - 1 + 'A');
        Terminate(null);
    }
}

ulong GetFirstDriveSectorForFileInFib()
{
	return *((ulong*)(driveInfo + 6));
}

ulong GetFirstFileSectorForFileInFib()
{
    ulong firstDataSector;
    byte sectorsPerCluster;

    regs.Words.DE = (int)driveParameters;
    regs.Bytes.L = fib->logicalDrive;
    DoDosCall(_DPARM);
    firstDataSector = *(uint*)(driveParameters+15);
    sectorsPerCluster = *(byte*)(driveParameters+3);
    
    return
        firstDataSector +
        ((fib->startCluster - 2) * sectorsPerCluster);
}

void AddFileInFibToFilesTable(ulong sector)
{
    GeneratedFileTableEntry* tableEntry;
    
    tableEntry =
        (GeneratedFileTableEntry*)
            (fileContentsBase + 
            sizeof(GeneratedFileHeader) + 
            (sizeof(GeneratedFileTableEntry) * totalFilesProcessed));
            
    tableEntry->deviceIndex = *(driveInfo + 4);
    tableEntry->logicalUnitNumber = *(driveInfo + 5);
    tableEntry->firstFileSector = sector;
    tableEntry->fileSizeInSector = (uint)(fib->fileSize >> 9);
}

void AddFileInFibToFilenamesInfo()
{
    int fileIndex = totalFilesProcessed + 1;
    
    sprintf(fileNamesAppendAddress, "%c -> ", fileIndex <= 9 ? fileIndex + '0' : fileIndex - 10 + 'A');
    fileNamesAppendAddress += 5;
    strcpy(fileNamesAppendAddress, fib->filename);
    fileNamesAppendAddress += strlen(fib->filename);
    *fileNamesAppendAddress++ = '\r';
    *fileNamesAppendAddress++ = '\n';
}

void GenerateFile() 
{
    GeneratedFileHeader* header;
    byte fileHandle;
    char* fileNamesHeader;
          
    if(bootFileIndex > totalFilesProcessed) {
        bootFileIndex = totalFilesProcessed;
        printf("\r\n*** Warning: boot file index is greater than number of files processed.\r\n    Set to %c instead in the generated file.\r\n",
            bootFileIndex <= 9 ? bootFileIndex + '0' : bootFileIndex - 10 + 'A');
    }
    
    header = (GeneratedFileHeader*)fileContentsBase;
    strcpy(header->signature, "Nextor DSK file");
    header->numberOfEntriesInImagesTable = totalFilesProcessed;
    header->indexOfImageToMountAtBoot = bootFileIndex;
    header->workAreaAddress = workAreaAddress;
    memset(header->reserved, 0, 4);
    
    regs.Words.DE = (int)outputFileName;
    regs.Bytes.A = 0;
    regs.Bytes.B = 0;
    DoDosCall(_CREATE);
    fileHandle = regs.Bytes.B;
    
    regs.Words.DE = (int)fileContentsBase;
    regs.Words.HL = 
            (uint)
            (sizeof(GeneratedFileHeader) + 
            (sizeof(GeneratedFileTableEntry) * totalFilesProcessed));
    DoDosCall(_WRITE);
    
    fileNamesHeader = "\fDisk image files registered:\r\n\r\n";
    regs.Bytes.B = fileHandle;
    regs.Words.DE = (int)fileNamesHeader;
    regs.Words.HL = strlen(fileNamesHeader);
    DoDosCall(_WRITE);
    
    regs.Bytes.B = fileHandle;
    regs.Words.DE = (int)fileNamesBase;
    regs.Words.HL = (int)(fileNamesAppendAddress - fileNamesBase);
    DoDosCall(_WRITE);
     
    regs.Bytes.B = fileHandle;
    DoDosCall(_CLOSE);

}

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
    regs.Bytes.B = 0x5A;
    regs.Words.HL = 0x1234;
    regs.Words.DE = 0xABCD;
    regs.Words.IX = 0;
    DosCall(_DOSVER, &regs, REGS_ALL, REGS_ALL);
	
    if(regs.Bytes.B < 2 || regs.Bytes.IXh != 1) {
        Terminate("This program is for Nextor only.");
    }
}

void* malloc(int size)
{
	void* value = mallocPointer;
	mallocPointer = (void*)(((int)mallocPointer) + size);
	return value;
}

uint ParseHex(char* hexString)
{
    uint result;
    char digit;
    
    result = 0;
    while((digit = *hexString) != 0) {
        digit |= 32;
        result *= 16;
        if(digit >= '0' && digit <= '9') {
            result += digit - '0';
        }
        else if(digit >= 'a' && digit <='f') {
            result += digit - 'a' + 10;
        }
        else {
            InvalidParameter();
        }
        hexString++;
    }
    
    return result;
}

void DoDosCall(byte functionCode)
{
    DosCall(functionCode, &regs, REGS_ALL, REGS_ALL);
    if(regs.Bytes.A != 0 && !(functionCode == _FNEXT && regs.Bytes.A == _NOFIL)) {
        TerminateWithDosError(regs.Bytes.A);
    }
}

void ResetComputer()
{
    regs.Bytes.IYh = *(byte*)EXPTBL;
    regs.Words.IX = 0;
    AsmCall(CALSLT, &regs, REGS_ALL, REGS_NONE);
}